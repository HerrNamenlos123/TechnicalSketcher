
#include "pch.h"
#include "Updater.h"
#include "Battery/AllegroDeps.h"

/*
 * In November 2021, the installer system was changed from Visual Studio installer projects 
 * to the WiX Toolchain. 
 *  
 * In older versions, there were updater.exe and uninstall.exe. Back then the updates were done
 * by update.exe, which was executed by the TechnicalSketcher. Now this is not needed anymore as
 * all updates are done by the TechnicalSketcher itself.
 * 
 * On existing installations, updater.exe is simply ignored now, it will not do anything as long 
 * as it is not executed and uninstall.exe is also not touched by newer versions, so it can be 
 * uninstalled properly with the original installer.
*/


#define UPDATE_MAX_ATTEMPTS 10		// Attempts for downloading
#define UPDATE_SLEEP_TIME 10		// Seconds to sleep between download attempts

#define TEMP_DIRECTORY "update/"
#define OUTDATED_DIRECTORY "outdated/"


std::shared_ptr<spdlog::logger> logger;

void DeleteOutdatedFiles(const std::string& rootdir) {
	if (Battery::RemoveDirectory(rootdir + OUTDATED_DIRECTORY)) {
		logger->info("Outdated files were deleted");
	}
}

std::pair<std::string, std::string> GetLatestGithubRelease(const std::string& url, int attempt = 0) {

	logger->info("Accessing api.github.com...");

	auto res = Battery::GetHttpRequest(url);
	if (!res.has_value()) {
		logger->info("Connection failed");

		if (attempt < UPDATE_MAX_ATTEMPTS) {
			Battery::Sleep(10);
			logger->info("Trying again");
			return GetLatestGithubRelease(url, attempt + 1);
		}
		else {
			logger->info("Too many attempts, aborting update process");
			return std::make_pair("", "");
		}
	}

	// Connection was valid
	std::string latestVersion;
	std::string downloadLink;
	try {
		nlohmann::json api = nlohmann::json::parse(res->body);
		latestVersion = api["tag_name"];

		for (nlohmann::json asset : api["assets"]) {
			if (asset["name"] == UPDATE_FILENAME) {
				downloadLink = asset["browser_download_url"];
			}
		}
	}
	catch (...) {}

	if (latestVersion.empty() || downloadLink.empty()) {
		logger->info("Failed to parse the JSON response");
		return std::make_pair("", "");
	}

	logger->info("Github response is valid");
	return std::make_pair(latestVersion, downloadLink);
}

bool DownloadUpdate(const std::string& url, const std::string& targetFile, int attempt = 0) {
	size_t cnt = 0;
	logger->info("Downloading {} to {}", url, targetFile);
	if (!Battery::DownloadUrlToFile(url, targetFile, true,
		[&](uint64_t progress, uint64_t total) {				// This is a lambda function!
			Navigator::GetInstance()->updateProgress = (double)progress / total;
			return true;
		}
	)) {
		logger->info("Download failed");

		if (attempt < UPDATE_MAX_ATTEMPTS) {
			Battery::Sleep(UPDATE_SLEEP_TIME);
			logger->info("Trying again...");
			DownloadUpdate(url, targetFile, attempt + 1);
		}
		else {
			logger->info("Too many attempts, stopping download");
			return false;
		}
	}

	return true;
}

void CleanUpdateFiles(const std::string& rootdir) {
	Battery::RemoveDirectory(rootdir + TEMP_DIRECTORY);
	Battery::RemoveFile(rootdir + UPDATE_FILENAME);
}

void RunUpdater() {

	Battery::Sleep(2);

	try {

		// This is the root directory in the installed localappdata folder
		//
		// - %localappdata%
		//   - TechnicalSketcher/  <-- rootdir
		//     - latest/
		//       - TechnicalSketcher.exe
		//       - icon.ico
		//	   - (outdated/)  - These two are used for updating
		//     - (update/)   
		//     - (updater.exe)	  - These two are there in existing installations of an older version
		//     - (uninstall.exe)  - (Not needed anymore)
		//
		std::string installDir = Battery::GetExecutableDirectory();
		std::string rootdir = Battery::GetParentDirectory(installDir);
		Navigator::GetInstance()->restartExecutablePath = Battery::GetExecutablePath();

		// Create the lockfile, only one process can update the application at a time
		std::unique_ptr<Battery::Lockfile> lockfile;
		try {
			lockfile = std::make_unique<Battery::Lockfile>(rootdir + "lock");
		}
		catch (...) {
			return;
		}

		// Setup logging
		std::string logfile = rootdir + UPDATER_LOG_FILENAME;
		Battery::RemoveFile(logfile);
		logger = spdlog::basic_logger_mt(UPDATER_LOGGER_NAME, logfile, true);
		logger->set_level(BATTERY_LOG_LEVEL_TRACE);




		// Delete old versions
		DeleteOutdatedFiles(rootdir);



		// Check if the application can be updated

		// First, get the currently installed version
		std::string version = Navigator::GetInstance()->GetApplicationVersion();
		logger->info("Installed version is {}", version);

		// Now check the latest version on Github
		std::pair<std::string, std::string> release = GetLatestGithubRelease(UPDATE_PACKAGE_URL);
		std::string latestVersion = release.first;
		std::string downloadLink = release.second;

		if (!latestVersion.empty()) {
			logger->info("Latest version available on GitHub is {}", latestVersion);
		}
		else {
			logger->info("Could not retrieve latest version tag from GitHub, aborting update process...");
			return;
		}



		// Check if update is necessary
		if (version == latestVersion) {
			logger->info("TechnicalSketcher is up-to-date, no update necessary");
			return;
		}
		else {
			logger->info("TechnicalSketcher can be updated, starting update process...");
		}

		// Now the updating process is official, notify the user
		Navigator::GetInstance()->updateStatus = UpdateStatus::INITIALIZING;
		Navigator::GetInstance()->updateProgress = 0.0;
		Battery::Sleep(0.5);

		// Download the update
		Navigator::GetInstance()->updateStatus = UpdateStatus::DOWNLOADING;
		logger->info("Downloading update package...");
		if (!DownloadUpdate(downloadLink, rootdir + UPDATE_FILENAME)) {
			logger->info("Failed to download update package, aborting update process...");
			Navigator::GetInstance()->timeSincePopup = Battery::GetRuntime();
			Navigator::GetInstance()->updateStatus = UpdateStatus::FAILED;
			return;
		}
		logger->info("Download was successful");

		// Unzip the update
		Navigator::GetInstance()->updateStatus = UpdateStatus::EXTRACTING;
		logger->info("Unzipping the update package...");
		if (!Battery::ExtractArchive(rootdir + UPDATE_FILENAME, rootdir + TEMP_DIRECTORY)) {
			logger->info("Failed to unzip update package, aborting update process...");
			Navigator::GetInstance()->timeSincePopup = Battery::GetRuntime();
			Navigator::GetInstance()->updateStatus = UpdateStatus::FAILED;
			CleanUpdateFiles(rootdir);
			return;
		}
		logger->info("Unzipping was successful");

		// Now move the old files to outdated
		Navigator::GetInstance()->updateStatus = UpdateStatus::INSTALLING;
		logger->info("Moving old files to outdated");	// This might fail when another outdated executable is still running
		if (Battery::DirectoryExists(installDir)) {
			std::string target = rootdir + OUTDATED_DIRECTORY;
			for (int i = 0; i < 10; i++) {
				if (Battery::MoveDirectory(installDir, target)) {
					break;
				}
				
				target += "temp/";
			}
		}
		else {
			logger->info("Application can't be updated: Executable directory is inconsistent!"
				" Aborting to avoid breaking the installation...");
			Navigator::GetInstance()->timeSincePopup = Battery::GetRuntime();
			Navigator::GetInstance()->updateStatus = UpdateStatus::FAILED;
			CleanUpdateFiles(rootdir);
			return;
		}
		// TODO: Fix screen refresh while updating

		// Now move the new files to the install directory
		logger->info("Moving new files to rootdir");
		if (!Battery::MoveDirectory(rootdir + TEMP_DIRECTORY, rootdir)) {
			logger->info("MoveDirectory failed, reverting installation...");
			Battery::PrepareEmptyDirectory(installDir);
			Battery::MoveDirectory(rootdir + OUTDATED_DIRECTORY, installDir);
			logger->info("Tried my best to recover the old installation, aborting update");
			Navigator::GetInstance()->timeSincePopup = Battery::GetRuntime();
			Navigator::GetInstance()->updateStatus = UpdateStatus::FAILED;
			CleanUpdateFiles(rootdir);
			return;
		}
		logger->info("Install directory was not called 'latest/', renaming to '{}/'", Battery::GetFilename(installDir));
		if (Battery::GetFilename(installDir) != "latest/") {
			Battery::RemoveDirectory(installDir);
			Battery::RenameFile(rootdir + "latest/", rootdir + Battery::GetFilename(installDir));
		}
		

		// Delete update package
		logger->info("Cleaning update files");
		CleanUpdateFiles(rootdir);

		logger->info("Update was successful, stopping update thread");

		Navigator::GetInstance()->updateStatus = UpdateStatus::DONE;
		Navigator::GetInstance()->timeSincePopup = Battery::GetRuntime();

		// This .exe file is still running in the outdated folder:
		// Will be deleted when the application is opened the next time

	}
	catch (const Battery::Exception& e) {
		LOG_CORE_CRITICAL(std::string("The Updater thread threw Battery::Exception: ") + e.what());
		Battery::ShowErrorMessageBox(std::string("The Updater thread threw Battery::Exception: ") + e.what());
		Navigator::GetInstance()->updateStatus = UpdateStatus::NOTHING;
		return;
	}
	catch (...) {
		LOG_CORE_CRITICAL("The Updater thread threw unknown Exception: No further information");
		Battery::ShowErrorMessageBox("The Updater thread threw unknown Exception: No further information");
		Navigator::GetInstance()->updateStatus = UpdateStatus::NOTHING;
		return;
	}
}
