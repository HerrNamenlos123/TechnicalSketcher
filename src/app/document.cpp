
#include "../app.h"

void addDocument(Appstate* appstate)
{
  Document document;
  document.paperColor = Color(255, 255, 255, 255);
  appstate->documents.push_back(std::move(document));
}
void addPageToDocument(Appstate* appstate, Document& document)
{
  Page page;
  document.pages.push_back(std::move(page));
}