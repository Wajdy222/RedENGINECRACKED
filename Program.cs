using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using Discord;
using Discord.Commands;
using Discord.WebSocket;
using Microsoft.Extensions.DependencyInjection;
using Newtonsoft.Json;
using zDisordBot.Modules;

namespace zDiscordBot
{
    class Program
    {
        static void Main(string[] args) => new Program().RunBotAsync().GetAwaiter().GetResult();

        public static DiscordSocketClient _client;
        private CommandService _commands;
        private IServiceProvider _services;

        public static ulong mainGuildId = 680572418561146969;

        public async Task RunBotAsync()
        {
            _client = new DiscordSocketClient();
            _commands = new CommandService();

            _services = new ServiceCollection()
                .AddSingleton(_client)
                .AddSingleton(_commands)
                .BuildServiceProvider();

            //string token = "Njc0NzQ1MTQzMzk5NTQ2ODgw.XjtFMg.AmXNyQXSxO1fNZtITQBROPyouG0";
            string token = "NjgwNTg3MzExMzc3MTU0MTQw.XlwnRQ.zkfOAYdmNXElEQMmw85CkgLsaZA";

            _client.Log += _client_Log;

            await RegisterCommandsAsync();

            await _client.LoginAsync(TokenType.Bot, token);

            await _client.StartAsync();

            await Task.Delay(-1);

        }

        private Task _client_Log(LogMessage arg)
        {
            Console.WriteLine(arg);
            return Task.CompletedTask;
        }

        public async Task RegisterCommandsAsync()
        {
            _client.MessageReceived += HandleCommandAsync;
            _client.UserJoined += AnnounceUserJoined;
            _client.UserLeft += AnnounceUserLeft;
            _client.Ready += ReadyTask;

            await _commands.AddModulesAsync(Assembly.GetEntryAssembly(), _services);
        }

        private async Task AnnounceUserJoined(SocketGuildUser user)
        {
            Console.WriteLine($"{user.Username} joined {user.Guild.Name}.");
            if (user.Guild.Id == mainGuildId)
            {
                dynamic response = JsonConvert.DeserializeObject(HttpRequest.Post("https://auth.prodigy.ovh/api/bot/v1", new NameValueCollection() {
                    { "botAction", "isUserBuyer" },
                    { "userid", user.Id.ToString() }
                }));

                if (user.IsBot || _client.GetGuild(mainGuildId).OwnerId == user.Id || user.GuildPermissions.Administrator)
                {
                    Console.WriteLine($"Bypassing {user.Username} because of checks");
                    return;
                }

                if (response.type == "success")
                {
                    try
                    {
                        Console.WriteLine($"Trying to add role memeber to {user.Username} in redENGINE customer discord");
                        var role = _client.GetGuild(mainGuildId).Roles.FirstOrDefault(x => x.Name == "Member");
                        await user.AddRoleAsync(role);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"Exception from buyer discord: {ex.Message}");
                        await user.SendMessageAsync("I couldn't add you the group but you're a buyer <3, ask a staff for help");
                    }
                }
                else
                    await user.KickAsync("You're not cool enough to be here");
            }
            else
            {
                IReadOnlyCollection<SocketGuildChannel> channels = _client.GetGuild(user.Guild.Id).Channels;

                foreach (SocketGuildChannel channel in channels)
                    if (channel.Name.Contains("landing-pad"))
                    {
                        await _client.GetGuild(user.Guild.Id).GetTextChannel(channel.Id).SendMessageAsync($"Hey {user.Username}! Thank you for joining our community :)", false);
                        break;
                    }
            }
            await Task.Delay(0);
        }

        private async Task AnnounceUserLeft(SocketGuildUser user)
        {
            IReadOnlyCollection<SocketGuildChannel> channels = _client.GetGuild(user.Guild.Id).Channels;

            foreach (SocketGuildChannel channel in channels)
                if (channel.Name.Contains("landing-pad"))
                {
                    await _client.GetGuild(user.Guild.Id).GetTextChannel(channel.Id).SendMessageAsync($"{user.Username} left us :sob:", false);
                    break;
                }
            await Task.Delay(0);
        }

        private async Task ReadyTask()
        {
            await _client.SetGameAsync("$commands for help", null, ActivityType.Playing);

            foreach(var user in _client.GetGuild(mainGuildId).Users)
            {
                var role = (user as IGuildUser).Guild.Roles.FirstOrDefault(x => x.Name.ToLower() == "squad");
                if (user.IsBot || _client.GetGuild(mainGuildId).OwnerId == user.Id || user.GuildPermissions.Administrator || user.Roles.Contains(role))
                    continue;

                dynamic response = JsonConvert.DeserializeObject(HttpRequest.Post("https://auth.prodigy.ovh/api/bot/v1", new NameValueCollection() {
                    { "botAction", "isUserBuyer" },
                    { "userid", user.Id.ToString() }
                }));

                if (response.type != "success")
                    await user.KickAsync("How are you even here bruh, you're not a buyer");
            }
        }

        private async Task HandleCommandAsync(SocketMessage arg)
        {
            try
            {
                List<string> blacklistedWords = new List<string>()
                {
                    "cheat",
                };

                foreach (string str in blacklistedWords)
                {
                    var user = (arg.Author as IGuildUser);
                    if (arg.Content.ToLower().Contains(str) && !user.GuildPermissions.Administrator)
                    {
                        await arg.DeleteAsync();
                        await arg.Channel.SendMessageAsync($"{arg.Author.Mention}, a word in that phrase was blacklisted. We're not trying to get clapped again :)");
                        return;
                    }
                }

                var message = (arg as SocketUserMessage);
                var context = new SocketCommandContext(_client, message);
                if (message.Author.IsBot) return;

                int argPos = 0;
                if (message.HasStringPrefix("$", ref argPos))
                {
                    var result = await _commands.ExecuteAsync(context, argPos, _services);
                    /*if (result.IsSuccess)
                    {
                        if(message.Channel.GetType() == typeof(SocketTextChannel))
                        {
                            SocketGuild guild = ((SocketGuildChannel)message.Channel).Guild;
                            IEmote emote = guild.Emotes.First(e => e.Name == "success");
                            await context.Message.AddReactionAsync(emote);
                        }
                        else
                            await context.Message.AddReactionAsync(new Emoji("✅"));
                    }
                    else
                    {
                        if (message.Channel.GetType() == typeof(SocketTextChannel))
                        {
                            SocketGuild guild = ((SocketGuildChannel)message.Channel).Guild;
                            IEmote emote = guild.Emotes.First(e => e.Name == "error");
                            await context.Message.AddReactionAsync(emote);
                        }
                        else
                            await context.Message.AddReactionAsync(new Emoji("⁉️"));
                    }*/
                }
            }
            catch (Exception) { }
            //var result = await _commands.ExecuteAsync(context, argPos, _services);
            //if (!result.IsSuccess) Console.WriteLine(result.ErrorReason);
        }
    }
}
