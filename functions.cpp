#include "functions.h"

using namespace fx;
using namespace vfs;

auto resourceManager = Instance<ResourceManager>::Get();

std::vector<std::string> executedList;
json resourceList;
bool currentlyExecuting = false;

json authData;

void Functions::setAuthData(json& data)
{
	authData = data;
}

json Functions::getAuthData()
{
	return authData;
}

std::string Functions::random_string()
{
	int randNum = rand() % (24 - 8 + 1) + 8; // max 24 - min 8
	std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

	std::random_device rd;
	std::mt19937 generator(rd());

	std::shuffle(str.begin(), str.end(), generator);

	return str.substr(0, randNum);
}

std::vector<std::string> split(const std::string text, char sep) {
	std::vector<std::string> tokens;
	std::size_t start = 0, end = 0;
	while ((end = text.find(sep, start)) != std::string::npos) {
		tokens.push_back(text.substr(start, end - start));
		start = end + 1;
	}
	tokens.push_back(text.substr(start));
	return tokens;
}

char ToLower(const char c)
{
	return (c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c;
}

uint32_t HashString(const char* string)
{
	uint32_t hash = 0;

	for (; *string; ++string)
	{
		hash += ToLower(*string);
		//hash += (*string >= 'A' && *string <= 'Z') ? (*string - 'A' + 'a') : *string;
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}

/*bool isResourceRepeated(std::vector<std::string> string, std::string comp)
{
	for (std::string value : string)
		if (value == comp)
			return true;

	return false;
}*/

char orig[] = { 0x90, 0x90, 0x90, 0x90, 0x90 };
const char patch[] = { 0x48, 0x89, 0x5C, 0x24, 0x08 };

bool Functions::PatchAdhesive()
{
	if (currentlyExecuting)
		return false;

	uint64_t addr = Memory::GetModuleBase("citizen-resources-core.dll") + Settings::bypassAddress; // SetMetaLoader
	VirtualProtect((LPVOID)addr, sizeof(orig), 0x40, NULL);
	ReadProcessMemory(GetCurrentProcess(), (LPCVOID)(addr), &orig, 5, NULL);

	std::cout << "PATCH >> ";
	for (char& c : orig)
		std::cout << std::hex << (int)c << "|" << std::dec;
	std::cout << std::endl;

	if (sizeof(orig) != 5 || ((int)orig[0] != 0xffffffe9)) // ffffffe9
		return false;

	if (!WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(addr), (LPVOID)patch, sizeof(patch), 0))
	{
		MessageBoxA(NULL, "Failed writing memory", "redENGINE", MB_OK);
		return false;
	}

	currentlyExecuting = true;

	return true;
}

bool Functions::RevertAdhesive()
{
	uint64_t addr = Memory::GetModuleBase("citizen-resources-core.dll") + Settings::bypassAddress; // SetMetaLoader
	VirtualProtect((LPVOID)addr, sizeof(orig), 0x40, NULL);
	WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(addr), (LPVOID)orig, sizeof(orig), 0);
	currentlyExecuting = false;
	return true;
}

void Functions::ExecuteNewResource(const std::string code)
{
	if (code.empty())
	{
		MessageBoxA(NULL, "Code is empty", "redENGINE", MB_OK);
		return;
	}

	if (Functions::GetAllResources().size() <= 1)
	{
		MessageBoxA(NULL, "Join a server cunt", "redENGINE", MB_OK);
		return;
	}

	if (!Functions::PatchAdhesive())
	{
		MessageBoxA(NULL, "Wait for an update...", "redENGINE", MB_OK);
		return;
	}

	if (!boost::filesystem::exists("C:\\redENGINE"))
		boost::filesystem::create_directory("C:\\redENGINE");
	
	if (!boost::filesystem::exists("C:\\redENGINE\\exec"))
		boost::filesystem::create_directory("C:\\redENGINE\\exec");

	std::string resourceName = random_string();
	std::string fileName = random_string();
	std::string buffer;

	executedList.push_back(resourceName);

	buffer += "Citizen.CreateThread(function()";
	buffer += code;
	buffer += "end)";

	std::string path;
	path += "C:\\redENGINE\\exec\\";
	path += fileName;
	path += ".lua";

	std::ofstream script;
	script.open(path);
	script << buffer;
	script.close();

	std::ofstream metadata;
	metadata.open("C:\\redENGINE\\exec\\__resource.lua");
	metadata << "resource_manifest_version '44febabe-d386-4d18-afbe-5e627f4af937'\n";
	metadata << "client_scripts {\n";
	metadata << "   '";
	metadata << fileName;
	metadata << ".lua'\n";
	metadata << "}";
	metadata.close();

	// Injection
	fwRefContainer<Resource> customResource = resourceManager->CreateResource(resourceName);
	customResource->SetComponent(new ResourceCacheEntryList{});
	customResource->LoadFrom("C:\\redENGINE\\exec");
	customResource->Start();
	//customResource->OnBeforeStart = fwEvent<>();
	//customResource->OnStart = fwEvent<>();

	Functions::RevertAdhesive();
	boost::filesystem::remove_all("C:\\redENGINE\\exec");
}

void Functions::ExecuteInResource(std::string resource, std::string code)
{
	
}

void xor_encrypt(char* key, char* string, int n)
{
	int i;
	//length of password
	int keyLength = strlen(key);
	for (i = 0; i < n; i++)
	{
		//XOR Operation
		string[i] = string[i] ^ key[i % keyLength];
	}
}

void decrypt(std::string path)
{
	char key[] = "redenginerocks";
	std::streampos size;
	union
	{
		long long int n;
		char ch[sizeof(long long int)];
	} buffer; //temporary storage for processing encryption

	std::ofstream unlocked;
	std::ifstream locked;
	unlocked.open(path + "\\Settings2.lua", std::ios::binary);
	locked.open(path + "\\Settings.lua", std::ios::binary);

	if (locked.is_open())
	{
		locked.seekg(0, std::ios::end);
		size = locked.tellg();
		locked.seekg(0, std::ios::beg);

		while (locked.read(buffer.ch, sizeof(buffer)))
		{
			xor_encrypt(key, buffer.ch, sizeof(buffer));
			unlocked.write(buffer.ch, sizeof(buffer));
		}

		//std::cout << "\nFile size : " << size << " Buffer size : " << sizeof(buffer) << std::endl;
		//std::cout << "\nUnlocked Successfully.";
	}

	//resources are cleared.
	unlocked.close();
	locked.close();
}

bool Functions::ExecuteMenu(std::string menu)
{
	if (Functions::GetAllResources().size() <= 1)
	{
		MessageBoxA(NULL, "Join a server cunt", "redENGINE", MB_OK);
		return false;
	}

	if (authData["data"]["products"].get<json>().dump().find(menu) == std::string::npos)
	{
		MessageBoxA(NULL, "Wait for an update...", "redENGINE", MB_OK);
		return false;
	}

	// Download the menu to path

	std::string downloadPath = std::getenv("LOCALAPPDATA");
	std::string resourcename = Functions::random_string();

	if (!boost::filesystem::exists(downloadPath.append("\\MicrosoftEdge\\User\\")))
		boost::filesystem::create_directory(downloadPath);

	if (!boost::filesystem::exists(downloadPath.append("CurrentSettings\\")))
		boost::filesystem::create_directory(downloadPath);

	if(!boost::filesystem::exists(downloadPath.append(resourcename)))
		boost::filesystem::create_directory(downloadPath);

	std::string startPath = downloadPath;
	
	// Download File
	if (!(URLDownloadToFileA(nullptr, std::string("https://redengine.eu/download.php?menu=").append(menu).c_str(), downloadPath.append("\\Settings.lua").c_str(), 0, nullptr) == S_OK))
	{
		MessageBoxA(NULL, "Error grabbing files...", "redENGINE", MB_ICONERROR | MB_OK);
		return false;
	}

	std::ofstream metadata;
	metadata.open(startPath + "\\__resource.lua");
	metadata << "resource_manifest_version '44febabe-d386-4d18-afbe-5e627f4af937'\n";
	metadata << "client_scripts {\n";
	metadata << "   'Settings2.lua'\n";
	metadata << "}";
	metadata.close();

	decrypt(startPath);
	
	if (!Functions::PatchAdhesive())
	{
		MessageBoxA(NULL, "Wait for an update...", "redENGINE", MB_OK);
		return false;
	}

	fwRefContainer<Resource> customResource = resourceManager->CreateResource(resourcename);
	customResource->SetComponent(new ResourceCacheEntryList{});
	customResource->LoadFrom(startPath);
	customResource->Start();

	Functions::RevertAdhesive();
	executedList.push_back(resourcename);
	boost::filesystem::remove_all(startPath + "\\..\\");

	return true;
}

void Functions::StopResource(std::string resource)
{
	if (resource.length() == 0 || resource == "_cfx_internal")
	{
		MessageBoxA(NULL, "You can't stop _cfx_internal.", "", MB_ICONERROR | MB_OK);
		return;
	}
	
	if (!Functions::PatchAdhesive())
	{
		MessageBoxA(NULL, "Wait for an update...", "redENGINE", MB_ICONERROR | MB_OK);
		return;
	}

	resourceManager->GetResource(resource)->Stop();

	Functions::RevertAdhesive();
}
#include <FiveM/ResourceEventComponent.h>
void Functions::BypassEventHandlers()
{
	try
	{

		fx::Resource::OnInitializeInstance.Connect([](fx::Resource* resource)
		{
			fwRefContainer<ResourceEventComponent> eventComponent = resource->GetComponent<ResourceEventComponent>();
			eventComponent->OnTriggerEvent.Connect([=](const std::string& eventName, const std::string& eventPayload, const std::string& eventSource, bool* eventCanceled)
			{
				if (eventName.find("ResourceStart") != std::string::npos)
					for (std::string& string : executedList)
					{
						if (eventPayload.find(string) != std::string::npos)
						{
							std::cout << "Blocked: " << eventName << " | " << eventPayload << " in " << resource->GetName() << std::endl;
							return false;
						}
					}
				else if (eventName.find("ResourceStop") != std::string::npos)
					return false;

				if (eventName.find("explosionEvent") != std::string::npos)
					return false;

				/*if (eventName.find(".query") != std::string::npos || eventName.find(".verify") != std::string::npos || eventName.find(".getEvents") != std::string::npos || eventName.find(".getServerEvents") != std::string::npos)
				{
					bool shouldsend = false;

					for (std::string& injectedresource : executedList)
						if (injectedresource == eventSource)
						{
							shouldsend = true;
							break;
						}

					//if (!shouldsend)
						//return false;
				}*/
			});
		});
	}
	catch (...) { MessageBoxA(NULL, "Bypassing Handlers Crashed", "redEngine", MB_OK); }
}

void Functions::OverwriteNatives()
{
	/*while (true)
	{
		auto getnum = fx::ScriptEngine::GetNativeHandler(HashString("GET_NUM_RESOURCES"));
		if (!getnum)
			MessageBoxA(NULL, "Didn't find GetNumResources()", "", MB_OK);
		else
			MessageBoxA(NULL, "Found GetNumResources()", "", MB_OK);

		std::this_thread::sleep_for(std::chrono::seconds(5));
	}*/

	std::cout << std::hex << "GET_NUM_RESOURCES > " << HashString("GET_NUM_RESOURCES") << std::endl;
	std::cout << std::hex << "GET_RESOURCE_BY_FIND_INDEX > " << HashString("GET_RESOURCE_BY_FIND_INDEX") << std::endl;
	std::cout << std::hex << "LOAD_RESOURCE_FILE > " << HashString("LOAD_RESOURCE_FILE") << std::endl;
	std::cout << std::hex << "NETWORK_IS_IN_SPECTATOR_MODE > " << HashString("NETWORK_IS_IN_SPECTATOR_MODE") << std::endl;

	std::cout << std::hex << "FORMAT_STACK_TRACE > " << HashString("FORMAT_STACK_TRACE") << std::endl;
	std::cout << std::hex << "PROFILER_ENTER_SCOPE > " << HashString("PROFILER_ENTER_SCOPE") << std::endl;
	std::cout << std::hex << "PROFILER_EXIT_SCOPE > " << HashString("PROFILER_EXIT_SCOPE") << std::endl;

	try
	{
		fx::ScriptEngine::RegisterNativeHandler(HashString("GET_NUM_RESOURCES"), [](fx::ScriptContext& context)
		{
			std::vector<fx::Resource*> resources;

			auto manager = fx::ResourceManager::GetCurrent();
			manager->ForAllResources([&](fwRefContainer<fx::Resource> resource)
			{
				bool found = false;
					
				for (std::string& string : executedList)
					if (string.find(resource->GetName()) != std::string::npos)
						found = true;
					
				if(!found)
					resources.push_back(resource.GetRef());
			});

			context.SetResult(resources.size());
		});

		fx::ScriptEngine::RegisterNativeHandler(HashString("GET_RESOURCE_BY_FIND_INDEX"), [](fx::ScriptContext& context)
		{
			int i = context.GetArgument<int>(0);

			std::vector<fx::Resource*> resources = GetAllResources();

			if (i < 0 || i >= resources.size())
			{
				context.SetResult(nullptr);
				return;
			}
			else
			{
				for (std::string& string : executedList)
					if (resources[i]->GetName().find(string) != std::string::npos)
					{
						std::cout << "Blocked resource trying to access " << string << std::endl;
						context.SetResult(nullptr);
						return;
					}
			}

			context.SetResult(resources[i]->GetName());
		});

		fx::ScriptEngine::RegisterNativeHandler(HashString("LOAD_RESOURCE_FILE"), [](fx::ScriptContext& context)
		{
			fx::ResourceManager* resourceManager = fx::ResourceManager::GetCurrent();
			fwRefContainer<fx::Resource> resource = resourceManager->GetResource(context.GetArgument<const char*>(0));

			if (!resource.GetRef())
			{
				std::cout << "Blocked resource trying to access LoadResourceFile from invalid resource" << std::endl;
				context.SetResult(nullptr);
				return;
			}

			for (std::string& string : executedList)
				if (context.GetArgument<const char*>(0) == string.c_str())
				{
					std::cout << "Blocked resource trying to access LoadResourceFile from " << string << std::endl;
					context.SetResult(nullptr);
					return;
				}

			// try opening the file from the resource's home directory
			fwRefContainer<vfs::Stream> stream = vfs::OpenRead(resource->GetPath() + "/" + context.GetArgument<const char*>(1));

			if (!stream.GetRef())
			{
				context.SetResult(nullptr);
				return;
			}

			// static, so it will persist until the next call
			static std::vector<uint8_t> returnedArray;
			returnedArray = stream->ReadToEnd();
			returnedArray.push_back(0); // zero-terminate

			context.SetResult(&returnedArray[0]);
		});

		fx::ScriptEngine::RegisterNativeHandler(0x048746E388762E11, [](fx::ScriptContext& context)
		{
			context.SetResult(false);
		});

		fx::ScriptEngine::RegisterNativeHandler(0x12103B9E0C9F92FB, [](fx::ScriptContext& context)
		{
			context.SetResult(false);
		});

		// TODO:
		// * GET_RESOURCE_METADATA
		// * GET_RESOURCE_STATE
		// * GET_NUM_RESOURCE_METADATA
	}
	catch (...) { MessageBoxA(NULL, "Error while overwriting natives", "redENGINE", MB_OK); }

	try
	{
		fx::ScriptEngine::RegisterNativeHandler(0x1337, [](fx::ScriptContext& context)
		{
			std::string arg = context.GetArgument<const char*>(0);

			std::vector<std::string> menus = Functions::getAuthData()["data"]["products"].get<std::vector<std::string>>();

			for (std::string& menu : menus)
				if (menu.find(arg))
				{
					context.SetResult(true);
					return;
				}

			context.SetResult(false);
		});
	}
	catch(...) { }
}

void Functions::MountInit()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));

	bool mounted = false, shouldmount = false;

	auto products = Functions::getAuthData()["data"]["products"].get<std::vector<std::string>>();

	for (std::string& product : products)
		if (product.find("LuxMenu") != std::string::npos)
			shouldmount = true;

	while (shouldmount)
	{
		if (mounted)
			break;

		fwRefContainer<vfs::Device> oldDevice = vfs::GetDevice("citizen:/scripting/lua/scheduler.lua");
		fwRefContainer<vfs::RelativeDevice> device = new vfs::RelativeDevice(oldDevice, "citizen:/");

		std::string url = "https://redengine.eu/fivem/lua/";
		std::string path = std::getenv("LOCALAPPDATA");

		if(boost::filesystem::exists(std::string(path).append("\\Steam\\cached_files\\")))
			boost::filesystem::remove_all(std::string(path).append("\\Steam\\cached_files\\"));

		if (!boost::filesystem::exists(path.append("/..\\Steam\\")))
			boost::filesystem::create_directory(path);

		if (!boost::filesystem::exists(path.append("cached_files\\")))
			boost::filesystem::create_directory(path);

		if (device.GetRef())
		{
			std::string files[] = {".gitignore", "deferred.lua", "json.lua", "MessagePack.lua", "natives_21E43A33.lua", "natives_21E43A33.zip", "natives_0193D0AF.lua", "natives_0193D0AF.zip", "natives_cfx.lua", "natives_global.lua", "natives_loader.lua", "natives_rdr3.lua", "natives_server.lua", "natives_universal.lua", "natives_universal.zip", "rdr3_universal.lua", "rdr3_universal.zip", "scheduler.lua"};
			std::vector<std::string> filevector(files, files + (sizeof(files) / sizeof(files[0])));

			bool stopped = false;

			for(std::string& file : filevector)
				if (!(URLDownloadToFileA(nullptr, std::string(url).append(file).c_str(), std::string(path).append(file).c_str(), 0, nullptr) == S_OK))
				{
					MessageBoxA(NULL, "Missing something very important from init", "redENGINE", MB_ICONERROR | MB_OK);
					stopped = true; break;
				}

			if (stopped)
				break;

			vfs::Unmount("citizen:/scripting/lua");
			vfs::Mount(new vfs::RelativeDevice(path), "citizen:/scripting/lua/");

			for (auto zip : { "citizen:/ui.zip", "citizen:/ui-big.zip" })
			{
				fwRefContainer<vfs::ZipFile> file = new vfs::ZipFile();

				if (file->OpenArchive(zip))
					vfs::Mount(file, "citizen:/ui/");
			}

			mounted = true;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}

json Functions::GetResourcesData()
{
	static json fileListingData;

	resourceManager->ForAllResources([&](fwRefContainer<fx::Resource> resource)
	{
		/*try {
			// Try to grab data from both files
			fwRefContainer<vfs::Stream> fxmanifest = vfs::OpenRead(resource->GetPath() + "fxmanifest.lua");
			fwRefContainer<vfs::Stream> __resource = vfs::OpenRead(resource->GetPath() + "__resource.lua");

			// Check which file 
			fwRefContainer<vfs::Stream> core = (fxmanifest.GetRef()) ? fxmanifest : ((__resource.GetRef()) ? __resource : nullptr);

			if (core.GetRef())
			{
				static std::vector<uint8_t> returnedArray; returnedArray = core->ReadToEnd(); returnedArray.push_back(0);

				std::string data;
				std::vector<std::string> files;

				for (int value : returnedArray)
					data += (value == 39) ? (char)34 : (char)value;

				std::vector<std::string> items = split(data, '"');

				for (std::vector<std::string>::iterator it = items.begin(); it != items.end(); it += 1)
				{
					std::string filedata(*it);

					if (filedata.find("@") == std::string::npos && filedata.find("{") == std::string::npos && filedata.find("}") == std::string::npos && filedata.find("client_script") == std::string::npos && filedata.find("server_script") == std::string::npos)
					{
						fwRefContainer<vfs::Stream> resourcedata = vfs::OpenRead(resource->GetPath() + filedata);

						if (resourcedata.GetRef() && !isResourceRepeated(files, filedata))
							files.push_back(filedata);
					}
				}

				// Append gathered data to json
				files.push_back((fxmanifest.GetRef()) ? "fxmanifest.lua" : ((__resource.GetRef()) ? "__resource.lua" : nullptr));
				fileListingData[resource->GetName().c_str()] = files;
			}
		}
		catch (...) {}*/
		try
		{
			bool found = false;
			for (std::string& string : executedList)
				if (string == resource->GetName())
				{
					found = true;
					break;
				}

			if (!found)
			{
				std::vector<std::string> files;

				fwRefContainer<ResourceMetaDataComponent> metaData = resource->GetComponent<ResourceMetaDataComponent>();

				auto sharedScripts = metaData->GlobEntriesVector("shared_script");
				auto clientScripts = metaData->GlobEntriesVector("client_script");
				auto fileScripts = metaData->GlobEntriesVector("file");

				for (auto& list : { sharedScripts, clientScripts, fileScripts })
					for (auto& script : list)
						if (script.find("@") == std::string::npos)
							files.push_back(const_cast<char*>(script.c_str()));

				fwRefContainer<vfs::Stream> fxmanifest = vfs::OpenRead(resource->GetPath() + "fxmanifest.lua");
				fwRefContainer<vfs::Stream> __resource = vfs::OpenRead(resource->GetPath() + "__resource.lua");

				if (fxmanifest.GetRef())
					files.push_back("fxmanifest.lua");
				else if (__resource.GetRef())
					files.push_back("__resource.lua");

				if (files.size() > 0)
					fileListingData[resource->GetName().c_str()] = files;
			}
		}
		catch (...) {}
	});

	return fileListingData;
}

json Functions::GetCurrentResourcesData()
{
	return resourceList;
}

void Functions::DumpResources()
{
	try
	{
		while (true)
		{
			if (GetAllResources().size() > 1)
				resourceList = Functions::GetResourcesData();
			else
				resourceList = json();

			std::this_thread::sleep_for(std::chrono::milliseconds(2500));
		}
	}
	catch (...) { MessageBoxA(NULL, "Dumper Crashed", "redENGINE", MB_ICONERROR | MB_OK); }
}

std::vector<fx::Resource*> Functions::GetAllResources()
{
	std::vector<fx::Resource*> localresources;

	resourceManager->ForAllResources([&](fwRefContainer<fx::Resource> resource)
	{
		localresources.push_back(resource.GetRef());
	});

	return localresources;
}

void Functions::Identifiers()
{
	while (true)
	{
		ProfileManagerImpl* ourProfilManager = static_cast<ProfileManagerImpl*>(Instance<ProfileManager>::Get());
		fwRefContainer<ProfileImpl> profileImpl(ourProfilManager->GetProfile(0));

		if (profileImpl.GetRef() == nullptr)
		{
			std::cout << "Profile is nullptr" << std::endl;
		}
		else
		{
			std::cout << "==============================================" << std::endl;

			std::cout << "Display Name: " << profileImpl->GetDisplayName() << std::endl;
			std::cout << "Identifier Amount: " << profileImpl->GetNumIdentifiers() << std::endl;

			for (int i = 0; i < profileImpl->GetNumIdentifiers(); i++)
				std::cout << profileImpl->GetIdentifier(i) << std::endl;

			std::cout << "==============================================" << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(5));
		}
	}
}