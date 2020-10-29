#pragma once

#include <Dependencies/websocketpp/config/asio_no_tls_client.hpp>
#include <Dependencies/websocketpp/client.hpp>
#include <Dependencies/nlohmann-json/json.hpp>
#include "memory.h"

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using json = nlohmann::json;

typedef websocketpp::client<websocketpp::config::asio_client> client;
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

namespace Auth
{
	namespace Internal
	{
		json userInfo;
		float version = 1.04;
		/*
		DATA TO BE SENT

			action    : string

			data
				private
					token       : string
					public_key  : string
					project     : string
				public
					username    : string
					password    : string
					identifiers : string

		DATA THAT THE CLIENT GATHERS AFTER AUTH

			auth : bool

			user
				admin : bool
				games : array
				menus : array
		*/

		bool first = true;
		std::string token;
		int key;

		std::string random_string(size_t length)
		{
			auto randchar = []() -> char
			{
				const char charset[] =
					"0123456789"
					"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
					"abcdefghijklmnopqrstuvwxyz";
				const size_t max_index = (sizeof(charset) - 1);
				return charset[rand() % max_index];
			};
			std::string str(length, 0);
			std::generate_n(str.begin(), length, randchar);
			return str;
		}

		bool IsProgramSafe()
		{
			// Checking for processes
			if (Memory::GetProcessID("HTTPDebuggerSvc.exe"))
				return false;
			else if (Memory::GetProcessID("HTTPDebuggerUI.exe"))
				return false;
			else if (Memory::GetProcessID("ida64.exe"))
				return false;

			// Check Hosts Files
			std::ifstream file("C:\\Windows\\System32\\drivers\\etc\\hosts");
			std::string str;

			while (std::getline(file, str))
				if (str.find("redengine.eu") != std::string::npos)
					return false;

			return true;
		}

		std::string str_xor(const std::string& data)
		{
			char key[] = { 'r', 'e', 'd', ' ', 'e', 'n', 'g', 'i', 'n', 'e', ' ', 'x', 'o', 'r' }; //Any chars will work
			std::string output = data;

			for (int i = 0; i < data.size(); i++)
				output[i] = data[i] ^ key[i % (sizeof(key) / sizeof(char))];

			return output;
		}

		std::string str_xor(const std::string& data, int key)
		{
			std::string output = data;

			for (int i = 0; i < data.size(); i++)
				output[i] = data[i] ^ key;

			return output;
		}

		// Functions
		void on_message(client* c, websocketpp::connection_hdl hdl, message_ptr msg)
		{
			try
			{
				json response;

				if (!first)
				{
					userInfo = json::parse(str_xor(str_xor(websocketpp::base64_decode(msg->get_payload())), key));

					if (userInfo["version"].get<float>() > version)
					{
						std::cout << "You need the latest update" << std::endl;
					}
					else
					{
						if (userInfo["type"].get<std::string>() == "success" && userInfo["data"]["token"].get<std::string>() == token && userInfo["data"]["products"].dump().find("FiveM") != std::string::npos)
							userInfo["auth"] = true;
					}

					userInfo["status"] = "online";

					c->close(hdl, websocketpp::close::status::normal, "");
				}
				else
				{
					key = std::stoi(msg->get_payload());
					token = Auth::Internal::random_string(16);
					response["action"] = "auth";
					response["data"]["private"]["token"] = token;
					response["data"]["private"]["project"] = "redengine";
					response["data"]["private"]["public_key"] = "TElpN2twRjNldGJMd0FqeEtQOEl3RExwSEZkc04zbg==";
					response["data"]["public"]["username"] = nullptr;
					response["data"]["public"]["password"] = nullptr;
					response["data"]["public"]["identifiers"] = Utils::GetCurrentUserSid();
					first = false;
					c->send(hdl, websocketpp::base64_encode(str_xor(str_xor(response.dump()), key)), websocketpp::frame::opcode::text);
				}
			}
			catch(json::exception&) { }
		}

		void on_open(client* c, websocketpp::connection_hdl hdl)
		{
			std::cout << "On Open" << std::endl;
		}

		void on_fail(client* c, websocketpp::connection_hdl hdl)
		{
			MessageBoxA(NULL, "Server is Offline", "redENGINE", MB_OK);
			std::cout << "Cannot connect to server" << std::endl;
		}

		void on_close(client* c, websocketpp::connection_hdl hdl)
		{
			std::cout << "On Close" << std::endl;
		}

		void Connect()
		{
			userInfo["auth"] = false;
			userInfo["status"] = "offline";

			if (!Auth::Internal::IsProgramSafe())
				return;

			client c;
			std::string uri = "ws://auth.redengine.eu:6969";
			//std::string uri = "ws://localhost:6969";
			try
			{
				c.set_access_channels(websocketpp::log::alevel::all);
				c.clear_access_channels(websocketpp::log::alevel::frame_payload);

				c.init_asio();

				c.set_open_handler(bind(&Auth::Internal::on_open, &c, ::_1));
				c.set_message_handler(bind(&Auth::Internal::on_message, &c, ::_1, ::_2));
				c.set_fail_handler(bind(&Auth::Internal::on_fail, &c, ::_1));
				c.set_close_handler(bind(&Auth::Internal::on_close, &c, ::_1));

				//c.clear_access_channels(websocketpp::log::alevel::all);
				//c.clear_error_channels(websocketpp::log::alevel::all);

				websocketpp::lib::error_code ec;
				client::connection_ptr con = c.get_connection(uri, ec);
				if (ec)
				{
					std::cout << "ec" << std::endl;
					std::cout << ec.message() << std::endl;
				}

				c.connect(con);
				c.run();
			}
			catch (websocketpp::exception const& e)
			{
				std::cout << "LOL" << std::endl;
				std::cout << e.m_msg << std::endl;
			}
			catch (const std::exception & e)
			{
				std::cout << "LOL2" << std::endl;
				std::cout << e.what() << std::endl;
			}
			catch (...)
			{
				std::cout << "other exception" << std::endl;
			}
		}
	}

	namespace User
	{
		bool HasAuthenticated()
		{
			return (Auth::Internal::userInfo["auth"].get<bool>());
		}

		bool IsUserAdmin()
		{
			return (Auth::Internal::userInfo["data"]["admin"].get<bool>());
		}

		bool OwnsMenu(std::string menu)
		{
			return (Auth::Internal::userInfo["data"]["menus"].get<json>().dump().find(menu) != std::string::npos);
		}

		bool OwnsGame(std::string game)
		{
			return (Auth::Internal::userInfo["data"]["games"].get<json>().dump().find(game) != std::string::npos);
		}
	}
}