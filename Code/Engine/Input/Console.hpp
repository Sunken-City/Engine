#pragma once
#include <vector>
#include <map>
#include <string>
#include "Engine\Renderer\RGBA.hpp"
#include "Engine\Core\Memory\UntrackedAllocator.hpp"
#include "Engine\Core\Memory\MemoryTracking.hpp"
#include "Engine\Core\Events\Event.hpp"

//-----------------------------------------------------------------------------------------------
#define UNUSED(x) (void)(x);

class Command;
class BitmapFont;
class Texture;
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
    bool RunCommand(const std::string& commandLine, bool addToHistory = false);
    bool RunCommand(const std::wstring& commandLine, bool addToHistory = false);
    inline bool IsActive() { return m_isActive; };
    inline bool IsEmpty() { return (m_cursorPointer == m_currentLine && *m_cursorPointer == '\0'); };
    static void RegisterCommand(const char* commandName, ConsoleCommandFunctionPointer consoleFunction);
    inline void BlinkCursor()
    {
        m_timeSinceCursorBlink = 0;
        m_renderCursor = true;
    };
    inline std::wstring GetCurrentWorkingDirectory() { return m_currentWorkingDirectory; };
    inline void SetCurrentWorkingDirectory(const std::wstring& newDirectory) { m_currentWorkingDirectory = newDirectory; };

    //VARIABLES//////////////////////////////////////////////////////////////////////////
    static Console* instance;
    BitmapFont* m_font = nullptr;
    Texture* m_backgroundTexture = nullptr;
    Event<> m_consoleUpdate;
    Event<> m_consoleClear;

private:
    //CONSTANTS//////////////////////////////////////////////////////////////////////////
    static const int MAX_LINE_LENGTH = 1024;
    static const int MAX_CONSOLE_LINES = 30;
    static const char CURSOR_CHARACTER;
    static const float CHARACTER_HEIGHT;
    static const float CHARACTER_WIDTH;
    static const float CURSOR_BLINK_RATE_SECONDS;

    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    std::vector<ColoredText*> m_consoleHistory;
    std::vector<std::wstring> m_commandHistory;
    std::wstring m_currentWorkingDirectory;
    char* m_currentLine;
    char* m_cursorPointer;
    char* m_leftmostSelectionCharacter = NULL;
    char* m_rightmostSelectionCharacter = NULL;
    float m_timeSinceCursorBlink;
    float m_timeSinceRepeatHeld = 0.0f;
    double m_timeLastActivatedMS = 0.0;
    int m_commandHistoryIndex = 0;
    int m_consoleHistoryIndex = 0;
    bool m_isActive;
    bool m_isCursorShowing;
    bool m_renderCursor = false;
};

//----------------------------------------------------------------------------------------------
class Command
{
public:
    Command(std::string fullCommandStr); //Split name and args into two buffers
    Command(std::wstring fullCommandStr); //Split name and args into two buffers
    inline std::string GetCommandName() const { return std::string(m_commandName.begin(), m_commandName.end()); };
    inline std::wstring GetWideCommandName() const { return m_commandName; };
    inline bool HasArgs(int argNumber) const { return m_argsList.size() == (unsigned int)argNumber; };
    inline std::string GetStringArgument(int argNumber) const { std::wstring argument = m_argsList[argNumber]; return std::string(argument.begin(), argument.end()); };
    inline std::wstring GetWStringArgument(int argNumber) const { return m_argsList[argNumber]; };
    inline int GetIntArgument(int argNumber) const { return std::stoi(m_argsList[argNumber]); };
    float GetFloatArgument(int argNumber) const { return std::stof(m_argsList[argNumber]); };
    inline std::string GetAllArguments() const { return std::string(m_fullArgsString.begin(), m_fullArgsString.end()); };
    inline std::wstring GetAllArgumentsWide() const { return m_fullArgsString; };

private:
    const std::wstring m_fullCommandStr;
    std::wstring m_commandName;
    std::wstring m_fullArgsString;
    std::vector<std::wstring> m_argsList;
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
