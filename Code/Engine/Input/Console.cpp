#include "Engine/Input/Console.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/Fonts/BitmapFont.hpp"
#include <cmath>
#include "../Core/StringUtils.hpp"
#include "InputOutputUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "../Time/Time.hpp"
#include "../Math/MathUtils.hpp"
#include "../Renderer/BufferedMeshRenderer.hpp"

Console* Console::instance = nullptr;
std::map<size_t, ConsoleCommandFunctionPointer, std::less<size_t>, UntrackedAllocator<std::pair<size_t, ConsoleCommandFunctionPointer>>>* g_consoleCommands = nullptr;
std::map<const char*, const char*, std::less<const char*>, UntrackedAllocator<std::pair<const char*, const char*>>>* g_helpStringLookup = nullptr;

const float Console::CHARACTER_HEIGHT = 20.0f;
const float Console::CHARACTER_WIDTH = 15.0f;
const float Console::CURSOR_BLINK_RATE_SECONDS = 0.5f;

//-----------------------------------------------------------------------------------
Console::Console()
    : m_currentLine(new char[MAX_LINE_LENGTH]())
    , m_cursorPointer(m_currentLine)
    , m_isActive(false)
    , m_isCursorShowing(false)
    , m_timeSinceCursorBlink(0.0f)
    , m_font(BitmapFont::CreateOrGetFont("FixedSys"))
    , m_commandHistoryIndex(0)
    , m_currentWorkingDirectory(RelativeToFullPath(L"."))
{
}

//-----------------------------------------------------------------------------------
Console::~Console()
{
    delete[] m_currentLine;
    ClearConsoleHistory();
}

//-----------------------------------------------------------------------------------
void Console::Update(float deltaSeconds)
{
    if (m_isActive)
    {
        m_consoleUpdate.Trigger();

        char currentChar = InputSystem::instance->GetLastPressedChar();
        ParseKey(currentChar);

        m_timeSinceCursorBlink += deltaSeconds;
        if (m_timeSinceCursorBlink >= CURSOR_BLINK_RATE_SECONDS)
        {
            m_timeSinceCursorBlink = 0.0f;
            m_renderCursor = !m_renderCursor;
        }

        int mousewheelDelta = InputSystem::instance->GetScrollDeltaThisFrame();
        if(mousewheelDelta != 0)
        {
            m_consoleHistoryIndex = Clamp<int>(m_consoleHistoryIndex - mousewheelDelta, 0, m_consoleHistory.size() - 1);
        }
    }
}

//-----------------------------------------------------------------------------------
void Console::ParseKey(char currentChar)
{
    bool controlHeldDown = InputSystem::instance->IsKeyDown(InputSystem::ExtraKeys::CTRL);
    bool isShiftDown = InputSystem::instance->IsKeyDown(InputSystem::ExtraKeys::SHIFT);

    if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::TAB))
    {
        //TODO: Make this work later.
//         std::string commandLine(m_currentLine);
//         std::vector<std::string>* tokenizedString = SplitString(commandLine, " ");
//         if (tokenizedString->size() == 1)
//         {
//             std::string partialCommandName = tokenizedString->at(0);
//         }
// 
//         delete tokenizedString;
    }

    if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::ESC))
    {
        if (IsEmpty())
        {
            DeactivateConsole();
            return;
        }
        else
        {
            m_cursorPointer = m_currentLine;
            memset(m_currentLine, 0x00, MAX_LINE_LENGTH);
        }
    }
    if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::BACKSPACE) && m_cursorPointer != m_currentLine)
    {
        --m_cursorPointer;
        int numCharactersToDelete = 1;

        if (controlHeldDown)
        {
            char* current = ++m_cursorPointer;
            while (!(current == m_currentLine || *current == ' '))
            {
                --current;
            }
            numCharactersToDelete = m_cursorPointer - current; 
            m_cursorPointer = current;
        }

        const int maxLengthInFrontOfCursor = MAX_LINE_LENGTH - (m_cursorPointer - m_currentLine);
        for (int i = 0; i < maxLengthInFrontOfCursor; i++)
        {
            char* currentIndex = m_cursorPointer + i;
            char* nextIndex = currentIndex + numCharactersToDelete;
            *currentIndex = *nextIndex;
            if (*nextIndex == '\0')
            {
                break;
            }
        }
        BlinkCursor();
    }
    else if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::ENTER))
    {
        if (IsEmpty())
        {
            DeactivateConsole();
            return;
        }
        std::string currentLine = std::string(m_currentLine);
        m_consoleHistory.push_back(new ColoredText(currentLine, RGBA::GRAY));
        if (!RunCommand(currentLine, true))
        {
            m_consoleHistory.push_back(new ColoredText("Invalid Command.", RGBA::MAROON));
        }
        m_cursorPointer = m_currentLine;
        m_consoleHistoryIndex = m_consoleHistory.size() - 1;
        memset(m_currentLine, 0x00, MAX_LINE_LENGTH);
        BlinkCursor();
    }
    else if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::LEFT) && m_cursorPointer != m_currentLine)
    {
        if (controlHeldDown)
        {
            char* current = --m_cursorPointer;
            while (!(current == m_currentLine || *current == ' '))
            {
                --current;
            }
            m_cursorPointer = current;
        }
        else
        {
            --m_cursorPointer;
        }
        BlinkCursor();
    }
    else if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::RIGHT) && m_cursorPointer != (m_currentLine + MAX_LINE_LENGTH))
    {
        if (controlHeldDown)
        {
            char* current = ++m_cursorPointer;
            while (!(current == '\0' || *current == ' '))
            {
                ++current;
            }
            m_cursorPointer = current;
        }
        else
        {
            ++m_cursorPointer;
        }
        BlinkCursor();
    }
    else if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::UP) && m_commandHistoryIndex > 0)
    {
        m_cursorPointer = m_currentLine;
        memset(m_currentLine, 0x00, MAX_LINE_LENGTH);
        --m_commandHistoryIndex;
        strcpy_s(m_currentLine, MAX_LINE_LENGTH, m_commandHistory[m_commandHistoryIndex].c_str());
        BlinkCursor();
    }
    else if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::DOWN) && m_commandHistoryIndex < m_commandHistory.size() - 1)
    {
        m_cursorPointer = m_currentLine;
        memset(m_currentLine, 0x00, MAX_LINE_LENGTH);
        ++m_commandHistoryIndex;
        strcpy_s(m_currentLine, MAX_LINE_LENGTH, m_commandHistory[m_commandHistoryIndex].c_str());
        BlinkCursor();
    }
    else if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::HOME))
    {
        m_cursorPointer = m_currentLine;
        BlinkCursor();
    }
    else if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::END))
    {
        for (int i = 0; i < MAX_LINE_LENGTH; i++)
        {
            char* currentIndex = m_cursorPointer + i;
            if (*currentIndex == '\0')
            {
                m_cursorPointer = currentIndex;
                break;
            }
        }
        BlinkCursor();
    }
    else if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::DEL))
    {
        int numCharactersToDelete = 1;
        if (controlHeldDown)
        {
            char* current = m_cursorPointer + 1;
            while (!(current == '\0' || *current == ' '))
            {
                ++current;
            }
            numCharactersToDelete = current - m_cursorPointer;
        }

        const int maxLengthInFrontOfCursor = MAX_LINE_LENGTH - (m_cursorPointer - m_currentLine);
        for (int i = 0; i < maxLengthInFrontOfCursor; i++)
        {
            char* currentIndex = m_cursorPointer + i;
            char* nextIndex = currentIndex + numCharactersToDelete;
            *currentIndex = *nextIndex;
            if (*nextIndex == '\0')
            {
                break;
            }
        }
        BlinkCursor();
    }
    else if (currentChar > 0x1F && m_cursorPointer != (m_currentLine + MAX_LINE_LENGTH))
    {
        if (*m_cursorPointer == '\0')
        {
            if ((m_cursorPointer + 1) != (m_currentLine + MAX_LINE_LENGTH))
            {
                *(m_cursorPointer + 1) = '\0';
            }
            else
            {
                return;
            }
        }
        *m_cursorPointer = currentChar;
        m_cursorPointer++;
        BlinkCursor();
    }
}

//-----------------------------------------------------------------------------------
void Console::Render() const
{
        float interpolationFactor = Clamp01((float)(GetCurrentTimeMilliseconds() - m_timeLastActivatedMS) / 100.0f);
        if (!m_isActive)
        {
            interpolationFactor = 1.0f - interpolationFactor;
        }
        Vector2 bottomLeft = Lerp<Vector2>(interpolationFactor, Vector2(0.0f, 450.0f), Vector2::ZERO);
        Vector2 topRight = Lerp<Vector2>(interpolationFactor, Vector2(1600.0f, 450.0f), Vector2(1600.0f, 900.0f));
        Renderer::instance->BeginOrtho(Vector2::ZERO, Vector2(1600.0f, 900.0f));
        {
            Renderer::instance->m_defaultMaterial->m_renderState.depthTestingMode = RenderState::DepthTestingMode::OFF;
            Renderer::instance->DrawAABB(AABB2(bottomLeft, topRight), RGBA(0x000000AA));
            if (m_backgroundTexture)
            {
                Renderer::instance->DrawTexturedAABB(AABB2(bottomLeft, topRight), Vector2(0.0f, 1.0f), Vector2(1.0f, 0.0f), m_backgroundTexture, RGBA::WHITE);
            }

            if (m_isActive && interpolationFactor > 0.9f)
            {
                Vector2 currentBaseline = Vector2::ONE * 10.0f;
                std::string currentLine = std::string(m_currentLine);
                if (m_renderCursor)
                {
                    int numCharsIntoString = (m_cursorPointer - m_currentLine) / sizeof(char);
                    AABB2 textBounds = m_font->CalcTextBounds(currentLine.substr(0, numCharsIntoString), 1.0f);
                    float currentTextWidth = textBounds.GetWidth();
                    float currentTextHeight = textBounds.GetHeight();
                    Vector3 cursorBottom(currentBaseline.x + currentTextWidth, currentBaseline.y, 0.0f);
                    Vector3 cursorTop(currentBaseline.x + currentTextWidth, currentBaseline.y + currentTextHeight, 0.0f);
                    Renderer::instance->DrawLine(cursorBottom, cursorTop, RGBA::WHITE, 2.0f);
                }

                BufferedMeshRenderer bufferedMeshRenderer;
                bufferedMeshRenderer.SetMaterial(m_font->GetMaterial());
                bufferedMeshRenderer.m_builder.AddText2D(currentBaseline, currentLine, 1.0f, RGBA::WHITE, true, m_font);

                int index = m_consoleHistoryIndex;
                unsigned int numberOfLinesPrinted = 0;
                for (auto reverseIterator = m_consoleHistory.rbegin(); reverseIterator != m_consoleHistory.rend(); ++reverseIterator, --index)
                {
                    if (index < 0)
                    {
                        break;
                    }
                    currentBaseline += Vector2(0.0f, (float)m_font->m_maxHeight);
                    bufferedMeshRenderer.m_builder.AddText2D(currentBaseline, m_consoleHistory[index]->text, 1.0f, m_consoleHistory[index]->color, true, m_font);
                    numberOfLinesPrinted++;
                    if (numberOfLinesPrinted > MAX_CONSOLE_LINES)
                    {
                        break;
                    }
                }
                bufferedMeshRenderer.FlushAndRender();
            }
            Renderer::instance->m_defaultMaterial->m_renderState.depthTestingMode = RenderState::DepthTestingMode::ON;
        }
        Renderer::instance->EndOrtho();
}

//-----------------------------------------------------------------------------------
void Console::ToggleConsole()
{
    m_isActive ? DeactivateConsole() : ActivateConsole();
    m_timeLastActivatedMS = GetCurrentTimeMilliseconds();
}

//-----------------------------------------------------------------------------------
void Console::ActivateConsole()
{
    m_isActive = true;
    m_timeLastActivatedMS = GetCurrentTimeMilliseconds();
}

//-----------------------------------------------------------------------------------
void Console::DeactivateConsole()
{
    m_isActive = false;
    m_timeLastActivatedMS = GetCurrentTimeMilliseconds();
}

//-----------------------------------------------------------------------------------
void Console::ClearConsoleHistory()
{
    for (ColoredText* text : m_consoleHistory)
    {
        delete text;
    }
    m_consoleHistory.clear();
    m_consoleHistoryIndex = 0;
    m_consoleClear.Trigger();
}

//-----------------------------------------------------------------------------------
void Console::RegisterCommand(const char* commandName, ConsoleCommandFunctionPointer consoleFunction)
{
    size_t commandNameHash = std::hash<std::string>{}(std::string(commandName));
    g_consoleCommands->emplace(commandNameHash, consoleFunction);
    g_helpStringLookup->emplace(commandName, "Write help text for this command! <3");
}

//-----------------------------------------------------------------------------------
void Console::PrintLine(std::string consoleLine, RGBA color)
{
    m_consoleHistory.push_back(new ColoredText(consoleLine, color));
    m_consoleHistoryIndex = m_consoleHistory.size() - 1;
}

//-----------------------------------------------------------------------------------
ColoredText* Console::PrintDynamicLine(std::string consoleLine, RGBA color /*= RGBA::WHITE*/)
{
    m_consoleHistory.push_back(new ColoredText(consoleLine, color));
    m_consoleHistoryIndex = m_consoleHistory.size() - 1;
    return *(--m_consoleHistory.end());
}

//-----------------------------------------------------------------------------------
//Returns true if command was found and run, false if invalid.
bool Console::RunCommand(const std::string& commandLine, bool addToHistory /*= false*/)
{
    if (addToHistory)
    {
        m_commandHistory.push_back(commandLine);
        m_commandHistoryIndex = m_commandHistory.size();
    }
    Command command(commandLine);

    size_t commandNameHash = std::hash<std::string>{}(command.GetCommandName());
    auto iterator = g_consoleCommands->find(commandNameHash);
    if (iterator != g_consoleCommands->end())
    {
        ConsoleCommandFunctionPointer outCommand = (*iterator).second;
        outCommand(command);
        return true;
    };
    return false;
}

//-----------------------------------------------------------------------------------
Command::Command(std::string fullCommandStr)
    : m_fullCommandStr(fullCommandStr)
    , m_fullArgsString("")
{
    char* charLine = (char*)fullCommandStr.c_str();
    char* token = nullptr;
    char* context = nullptr;
    char delimiters[] = " ";

    token = strtok_s(charLine, delimiters, &context);
    //First "arg" is the name
    if (token == nullptr)
    {
        m_commandName = std::string("INVALID_COMMAND");
    }
    else
    {
        m_commandName = std::string(token);
        std::transform(m_commandName.begin(), m_commandName.end(), m_commandName.begin(), ::tolower);
        token = strtok_s(NULL, delimiters, &context);

        //Save off the full list of arguments in case we forward this command.
        if (token)
        {
            m_fullArgsString = m_fullCommandStr.substr(m_fullCommandStr.find(" ") + 1);
        }

        bool isInQuotes = false;
        while (token != nullptr)
        {
            if (isInQuotes)
            {
                int tokenLength = strlen(token);
                unsigned int size = m_argsList.size();
                std::string currentString = m_argsList[size - 1];
                std::string addition = std::string(token);

                if (token[tokenLength - 1] == '"')
                {
                    isInQuotes = false;
                    addition = addition.substr(0, addition.length() - 1);
                }

                m_argsList[size - 1] = currentString + " " + addition;
                token = strtok_s(NULL, delimiters, &context);

                continue;
            }

            if (token[0] == '"')
            {
                isInQuotes = true;
                std::string tokenString(token);
                tokenString = tokenString.substr(1);
                m_argsList.push_back(tokenString);
            }
            else
            {
                m_argsList.push_back(std::string(token));
            }
            token = strtok_s(NULL, delimiters, &context);
        }
    }
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(help)
{
    if (args.HasArgs(0))
    {
        Console::instance->PrintLine("Console Controls:", RGBA::WHITE);
        Console::instance->PrintLine("Enter ~ Run command / Close console (if line empty)", RGBA::GRAY);
        Console::instance->PrintLine("All registered commands:", RGBA::WHITE);
        float i = 0.0f;
        for (auto helpStringPair : *g_helpStringLookup)
        {
            float frequency = 3.14f * 2.0f / (float)g_helpStringLookup->size();
            float center = 0.5f;
            float width = 0.49f;
            float red = sin(frequency * i + 2.0f) * width + center;
            float green = sin(frequency * i + 0.0f) * width + center;
            float blue = sin(frequency * i + 4.0f) * width + center;
            Console::instance->PrintLine(std::string(helpStringPair.first), RGBA(red, green, blue));
            i += 1.0f;
        }
        return;
    }
    if (!args.HasArgs(1))
    {
        Console::instance->PrintLine("help <string>", RGBA::GRAY);
        return;
    }
    std::string arg0 = args.GetStringArgument(0);
#pragma todo("Make this actually work, we're trying to look up the help text but this is doing a pointer comparison not strcmp")
    auto iter = g_helpStringLookup->find(arg0.c_str());
    if (iter != g_helpStringLookup->end())
    {
        Console::instance->PrintLine(iter->first, RGBA::GRAY);
    }
    if (arg0 == "help")
    {
        Console::instance->PrintLine("help: A command (that you just used) to find more info on other commands! Success! :D", RGBA::GRAY);
    }
    else if (arg0 == "clear")
    {
        Console::instance->PrintLine("clear: Clears the command history for the console", RGBA::GRAY);
    }
    else if (arg0 == "quit")
    {
        Console::instance->PrintLine("quit: Quits the application after saving any data.", RGBA::GRAY);
    }
    else if (arg0 == "motd")
    {
        Console::instance->PrintLine("motd: Displays the Message of the Day", RGBA::GRAY);
    }
    else if (arg0 == "runfor")
    {
        Console::instance->PrintLine("runfor: Runs a no-arg command for the specified number of times. Only used for sillyness.", RGBA::GRAY);
    }
    else if (arg0 == "changefont")
    {
        Console::instance->PrintLine("changefont: Changes the console's default font to a named font from the font folder.", RGBA::GRAY);
    }
    else
    {
        Console::instance->PrintLine("Undocumented or Unknown command", RGBA::GRAY);
    }
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(clear)
{
    UNUSED(args)
    Console::instance->ClearConsoleHistory();
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(quit)
{
    UNUSED(args)
    Console::instance->PrintLine("Saving and shutting down...", RGBA::RED);
    g_isQuitting = true;
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(runfor)
{
    if (!args.HasArgs(2))
    {
        Console::instance->PrintLine("runfor <# of Times to Run> <command name>", RGBA::GRAY);
        return;
    }
    int numberOfTimesToRun = args.GetIntArgument(0);
    std::string commandName = args.GetStringArgument(1);
    for (int i = 0; i < numberOfTimesToRun; i++)
    {
        Console::instance->RunCommand(commandName);
    }
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(changefont)
{
    if (!args.HasArgs(1))
    {
        Console::instance->PrintLine("changefont <fontName>", RGBA::GRAY);
        return;
    }
    std::string fontName = args.GetStringArgument(0);
    BitmapFont* font = BitmapFont::CreateOrGetFont(fontName);
    if (font)
    {
        Console::instance->m_font = font;
        Console::instance->PrintLine(fontName + " successfully loaded!", RGBA::FOREST_GREEN);
    }
    else
    {
        Console::instance->PrintLine("Font not found", RGBA::MAROON);
    }
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(dir)
{
    std::wstring wideCWD = Console::instance->GetCurrentWorkingDirectory();
    std::string cwd = std::string(wideCWD.begin(), wideCWD.end());
    std::vector<std::string> files = EnumerateFiles(cwd, "*");

    Console::instance->PrintLine(Stringf("Files in %s:", cwd.c_str()), RGBA::JOLTIK_YELLOW);
    for (std::string& string : files)
    {
        Console::instance->PrintLine(string, RGBA::JOLTIK_PURPLE);
    }
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(ls)
{
    Console::instance->RunCommand("dir");
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(cd)
{
    if (args.HasArgs(0))
    {
        Console::instance->PrintLine("cd <newDirectory>", RGBA::GRAY);
        return;
    }
    std::string newDirectory = args.GetAllArguments();

    std::wstring cwd = Console::instance->GetCurrentWorkingDirectory();
    std::wstring wideNewCWD = cwd + L"\\" + std::wstring(newDirectory.begin(), newDirectory.end());

    if (!DirectoryExists(wideNewCWD))
    {
        Console::instance->PrintLine("Directory doesn't exist.", RGBA::RED);
        return;
    }

    wideNewCWD = RelativeToFullPath(wideNewCWD); //Lazily remove any /.. we append to the path.
    Console::instance->SetCurrentWorkingDirectory(wideNewCWD);
    Console::instance->PrintLine(Stringf("Changed directories to %s", std::string(wideNewCWD.begin(), wideNewCWD.end()).c_str()), RGBA::KHAKI);
}