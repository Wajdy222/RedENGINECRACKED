#include "menu.h"

// Vars for ImGui
bool firsttime = true;
static const char* current_resource = NULL;
static const char* current_menu = NULL;
std::string currentDumpResource;
auto resourceManagerMenu = Instance<fx::ResourceManager>::Get();

std::string fromUint8_t(uint8_t key[], size_t size) {

	std::ostringstream ss;
	std::copy(key, key + size, std::ostream_iterator<char>(ss));
	return ss.str();
}

void Menu::Init()
{
	editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
	editor.SetShowWhitespaces(false);
	editor.SetPalette(TextEditor::GetRedEnginePalette());
	editor.SetReadOnly(false);

	dump.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
	dump.SetShowWhitespaces(false);
	dump.SetPalette(TextEditor::GetRedEnginePalette());
	dump.SetReadOnly(true);
	
	firsttime = !firsttime;
}

void Menu::Render()
{
	std::string username = Functions::getAuthData()["data"]["username"].get<std::string>();

	if (firsttime)
		Menu::Init();

	ImGui::Begin(std::string("redENGINE Executor | ").append(username).c_str(), NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
	{
		auto size = ImGui::GetWindowSize();

		editor.Render("TextEditor", ImVec2(size.x - 16, size.y - 56), true); ImGui::Spacing();

		if (ImGui::Button("Execute"))
			Functions::ExecuteNewResource(editor.GetText());
		if(ImGui::IsItemHovered())
			ImGui::SetTooltip("Executes a new resource on the server.");
		ImGui::SameLine();
		if (ImGui::Button("Clear"))
			editor.SetText("");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Clears the code from the window.");
		ImGui::SameLine();
		if (ImGui::Button("Open"))
			ImGuiFileDialog::Instance()->OpenDialog("fileKey", "Choose File", ".lua\0\0", ".");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Lets you choose a lua file. If file is over 400kb it's executed automatically.");
		ImGui::SameLine();

		if (ImGui::BeginCombo("##cheatlist", current_resource))
		{
			for(fx::Resource* resource : Functions::GetAllResources())
			{
				if (resource->GetState() == fx::ResourceState::Started)
				{
					bool is_selected = (current_resource == resource->GetName().c_str());
					if (ImGui::Selectable(resource->GetName().c_str(), is_selected))
					{
						current_resource = resource->GetName().c_str();
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
				}
			}
			ImGui::EndCombo();
		} ImGui::SameLine();
		if (ImGui::Button("Stop"))
			if (current_resource != NULL)
				Functions::StopResource(current_resource);
			else
				MessageBoxA(NULL, "Select a Resource", "redENGINE", MB_OK);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Stops the selected resource.");
		//ImGui::Checkbox("Anti-Cheat Bypass ()", &Menu::Options::anticheatBypass);

		if (ImGuiFileDialog::Instance()->FileDialog("fileKey"))
		{
			if (ImGuiFileDialog::Instance()->IsOk == true)
			{
				std::fstream file;
				std::string filedata, data;

				std::string filePath = ImGuiFileDialog::Instance()->GetFilepathName();

				// Get Filesize
				std::ifstream filesize(filePath, std::ios::binary);
				long begin = 0, end = 0;
				begin = filesize.tellg();
				filesize.seekg(0, std::ios::end);
				end = filesize.tellg();
				filesize.close();
				// =====================

				file.open(filePath);

				while (std::getline(file, data))
					filedata.append(data).append("\n");
				if ((end - begin) > 400000)
					Functions::ExecuteNewResource(filedata);
				else
					editor.SetText(filedata);
			}
			ImGuiFileDialog::Instance()->CloseDialog("fileKey");
		}
	}
	ImGui::End();

	ImGui::Begin(std::string("redENGINE Dumper | ").append(username).c_str(), NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
	{
		auto size = ImGui::GetWindowSize();
		json data = Functions::GetCurrentResourcesData();
		ImGui::BeginChild("##dumpResources", ImVec2(200, size.y - 56), true);
		{
			for (json::iterator it = data.begin(); it != data.end(); ++it)
			{
				if(ImGui::TreeNode(it.key().c_str()))
				{
					try
					{
						for (auto& element : it.value())
						{
							if ((element.get<std::string>().find(".png") == std::string::npos && element.get<std::string>().find(".jpg") == std::string::npos && element.get<std::string>().find(".min.css") == std::string::npos && element.get<std::string>().find(".min.js") == std::string::npos && element.get<std::string>().find(".wof") == std::string::npos && element.get<std::string>().find(".ttf") == std::string::npos) && ImGui::Button(element.get<std::string>().c_str()))
							{
								currentDumpResource.clear();
								currentDumpResource.append(it.key()).append("/").append(element.get<std::string>());

								if (currentDumpResource.find(".lua") != std::string::npos)
									dump.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
								else if (currentDumpResource.find(".sql") != std::string::npos)
									dump.SetLanguageDefinition(TextEditor::LanguageDefinition::SQL());
								else
									dump.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());

								fwRefContainer<vfs::Stream> resourcedata = vfs::OpenRead(resourceManagerMenu->GetResource(it.key())->GetPath() + element.get<std::string>());
								if (resourcedata.GetRef())
								{
									static std::vector<uint8_t> returnedData; returnedData = resourcedata->ReadToEnd(); returnedData.push_back(0);

									dump.SetText(reinterpret_cast<char*>(&returnedData[0]));
								}
								else
									dump.SetText("-- Failed to dump this file.");
							}
						}
					}
					catch(...) { }
					ImGui::TreePop();
				}
			}
		}
		ImGui::EndChild(); ImGui::SameLine();

		ImGui::BeginChild("##dumpEditor", ImVec2(size.x - 224, size.y - 56), false);
		{
			dump.Render("TextEditor", ImVec2(), true);
		}
		ImGui::EndChild();

		if (ImGui::Button("Save All Resources", ImVec2(200, 19)))
		{
			try
			{
				if (Functions::GetAllResources().size() <= 1)
					MessageBoxA(NULL, "Join a server before dumping nigga", "redENGINE", MB_OK);
				else
				{
					std::string foldername = "C:\\redENGINE_Dumps\\";

					auto start = std::chrono::high_resolution_clock::now();

					//_mkdir(foldername.c_str());
					boost::filesystem::create_directory(foldername);
					boost::filesystem::create_directory(foldername.append(Functions::random_string()));

					for (json::iterator it = data.begin(); it != data.end(); ++it)
						for (auto& element : it.value())
						{
							fwRefContainer<vfs::Stream> resourcedata = vfs::OpenRead(resourceManagerMenu->GetResource(it.key())->GetPath() + element.get<std::string>());

							if (resourcedata.GetRef())
							{
								static std::vector<uint8_t> returnedData; returnedData = resourcedata->ReadToEnd();

								std::string outputFolder(foldername);
								// Create Folder for ResourceName
								if (!boost::filesystem::exists(outputFolder.append("\\").append(it.key())))
									boost::filesystem::create_directory(outputFolder);

								std::string s(element.get<std::string>());
								std::string delimiter = "/";

								size_t pos = 0;
								std::string token;
								while ((pos = s.find(delimiter)) != std::string::npos)
								{
									token = s.substr(0, pos);
									outputFolder.append("\\").append(token);

									std::string foldercheck(foldername);
									if (!boost::filesystem::exists(outputFolder))
										boost::filesystem::create_directory(outputFolder);

									//std::cout << "Token: " << outputFolder << std::endl;
									s.erase(0, pos + delimiter.length());
								}

								// Create the file and set its buffer
								std::ofstream file((pos == 0) ? outputFolder.append("\\").append(element.get<std::string>()) : outputFolder.append("\\").append(s), std::ios::binary);

								file.write(reinterpret_cast<char*>(&returnedData[0]), returnedData.size() * sizeof(returnedData[0]));

								//std::cout << outputFolder << "|" << returnedData.size() << std::endl;
								file.close();
							}
						}

					auto end = std::chrono::high_resolution_clock::now();

					std::stringstream ss;
					ss << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

					std::string message("Finished dumping in ");
					message.append(ss.str()).append("ms (").append(foldername).append(")");

					MessageBoxA(NULL, message.c_str(), "redENGINE", MB_OK);
				}
			}
			catch(...) { }
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Saves all resources locally.");
	}
	ImGui::End();

	try
	{
		std::vector<std::string> menus = Functions::getAuthData()["data"]["products"].get<std::vector<std::string>>();

		int menuCount = 0;
		for (std::string& menu : menus)
			if (menu.find("menu_") != std::string::npos)
				menuCount++;

		if (menuCount > 0) //  && menuData["data"]["admin"].get<bool>()
		{
			ImGui::Begin(std::string("redENGINE Menus | ").append(username).c_str(), NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
			{
				ImGui::BeginChild("##menusChild", ImVec2(), true);
				{
					for (auto& menu : menus)
						if (menu.find("menu_") != std::string::npos)
						{
							std::string thismenu = menu;
							thismenu.replace(menu.find("menu_"), 5, "");
							std::string menumsg("Execute ");
							if (ImGui::Button(menumsg.append(thismenu).c_str(), ImVec2(200, 19)))
								Functions::ExecuteMenu(menu);
						}
				}
				ImGui::EndChild();
			}
			ImGui::End();
		}
	}
	catch (...) { std::cout << "Exception handler: " << Functions::getAuthData().dump() << std::endl; }
}