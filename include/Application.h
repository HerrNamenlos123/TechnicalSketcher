#pragma once

#include "Battery.h"

class Application : public Battery::Engine {
public:
	
	bool Setup() override {

		//Battery::Updater::CheckForGithubUpdate("HerrNamenlos123", "TechnicalSketcher");
		Battery::NetUtils::DownloadFile("https://github.com/HerrNamenlos123/TechnicalSketcher/releases/download/v0.1-alpha.3/TechnicalSketcher-Setup-x64.msi"
			, "testFile.bin", "cacert.pem");
		
		/*std::cout << "Filesize: " << (int64_t)Battery::NetUtils::GetSizeOfDownloadFile(
			"https://github.com/HerrNamenlos123/TechnicalSketcher/releases/download/v0.1-alpha.3/TechnicalSketcher-Setup-x64.msi",
			"cacert.pem", "gister") << std::endl;*/

		return true;
	}

	bool Update() override {
		using namespace Battery::Graphics;
		
		DrawBackground({ 0, 0, 0 });

		ImGui::Begin("Test");
		ImGui::End();

		return true;
	}

	void Shutdown() override {

	}

};
