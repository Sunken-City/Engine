#pragma once
#include <vector>
#include <map>
#include <string>
#include "Engine\Renderer\RGBA.hpp"
#include "Engine\Core\Memory\UntrackedAllocator.hpp"
#include "Engine\Core\Memory\MemoryTracking.hpp"
#include "Engine\Core\Event.hpp"

//-----------------------------------------------------------------------------------------------
#define UNUSED(x) (void)(x);

class Command;
class BitmapFont;
typedef void(*ConsoleCommandFunctionPointer)(Command&); 

//Used for quitting the application, bound to our Main_Win32.cpp; remove this if we aren't using it anymore.
extern bool g_isQuitting;
extern std::map<size_t, ConsoleCommandFunctionPointer, std::less<size_t>, UntrackedAllocator<std::pair<size_t, ConsoleCommandFunctionPointer>>>* g_consoleCommands;
extern std::map<const char*, const char*, std::less<const char*>, UntrackedAllocator<std::pair<const char*, const char*>>>* g_helpStringLookup;

//----------------------------------------------------------------------------------------------
struct ColoredText
{
    ColoredText(std::string line, RGBA tint) : text(line), color(tint) {};
    std::string text;
    RGBA color;
};

//----------------------------------------------------------------------------------------------
class Console
{
public:
    //CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
    Console();
    ~Console();

    //FUNCTIONS//////////////////////////////////////////////////////////////////////////
    void Update(float deltaSeconds);
    void Render() const;
    void ParseKey(char currentChar);
    void ToggleConsole();
    void ActivateConsole();
    void DeactivateConsole(); 
    void ClearConsoleHistory();
    void PrintLine(std::string consoleLine, RGBA color = RGBA::WHITE);
    ColoredText* PrintDynamicLine(std::string consoleLine, RGBA color = RGBA::WHITE);
    bool RunCommand(const std::string& commandLine);
    inline bool IsActive() { return m_isActive; };
    inline bool IsEmpty() { return (m_cursorPointer == m_currentLine && *m_cursorPointer == '\0'); };
    static void RegisterCommand(const char* commandName, ConsoleCommandFunctionPointer consoleFunction);

    //VARIABLES//////////////////////////////////////////////////////////////////////////
    static Console* instance;
    BitmapFont* m_font;
    Event<> m_consoleUpdate;
    Event<> m_consoleClear;

private:
    //CONSTANTS//////////////////////////////////////////////////////////////////////////
    static const int MAX_LINE_LENGTH = 128;
    static const int MAX_CONSOLE_LINES = 30;
    static const char CURSOR_CHARACTER;
    static const float CHARACTER_HEIGHT;
    static const float CHARACTER_WIDTH;
    static const float CURSOR_BLINK_RATE_SECONDS;

    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    bool m_isActive;
    bool m_isCursorShowing;
    char* m_currentLine;
    char* m_cursorPointer;
    char m_characterAtCursor;
    float m_timeSinceCursorBlink;
    unsigned int m_commandHistoryIndex;
    std::vector<ColoredText*> m_consoleHistory;
    std::vector<std::string> m_commandHistory;
};

//----------------------------------------------------------------------------------------------
class Command
{
public:
    Command(std::string fullCommandStr); //Split name and args into two buffers
    inline std::string GetCommandName() const { return m_commandName; };
    inline bool HasArgs(int argNumber) const { return m_argsList.size() == (unsigned int)argNumber; };
    inline std::string GetStringArgument(int argNumber) const { return m_argsList[argNumber]; };
    inline int GetIntArgument(int argNumber) const { return std::stoi(m_argsList[argNumber]); };
    float GetFloatArgument(int argNumber) const { return std::stof(m_argsList[argNumber]); };
    inline std::string GetAllArguments() const { return m_fullArgsString; };

private:
    const std::string m_fullCommandStr;
    std::string m_commandName;
    std::string m_fullArgsString;
    std::vector<std::string> m_argsList;
};

//----------------------------------------------------------------------------------------------
class RegisterCommandHelper
{
public:
    RegisterCommandHelper(const char* name, ConsoleCommandFunctionPointer command)
    {
        if (!g_consoleCommands)
        {
            g_consoleCommands = UntrackedNew<std::map<size_t, ConsoleCommandFunctionPointer, std::less<size_t>, UntrackedAllocator<std::pair<size_t, ConsoleCommandFunctionPointer>>>>();
        }
        if (!g_helpStringLookup)
        {
            g_helpStringLookup = UntrackedNew<std::map<const char*, const char*, std::less<const char*>, UntrackedAllocator<std::pair<const char*, const char*>>>>();
        }
        Console::RegisterCommand(name, command);
    }
};

//Macro that allows us to define a console command from anywhere
#define CONSOLE_COMMAND(name) void ConsoleCommand_ ## name ## ( Command & args ); \
    static RegisterCommandHelper RegistrationHelper_ ## name ##( #name, ConsoleCommand_ ## name ## ); \
    void ConsoleCommand_ ## name ##(Command &args)
