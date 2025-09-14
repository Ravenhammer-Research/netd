/*
 * Copyright (c) 2024 Paige Thompson / Ravenhammer Research (paige@paige.bio)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef NETD_CLIENT_TUI_HPP
#define NETD_CLIENT_TUI_HPP

#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <client/include/netconf/client.hpp>

#define MAX_LINES 1000

namespace netd::client::tui {

  class TUI {
  public:
    TUI();
    ~TUI();

    // TUI control
    bool initialize();
    void cleanup();
    bool isInitialized() const { return initialized_; }

    // Input/Output
    std::string readLine();
    void putLine(const std::string &text);
    void clear();

    // Command history
    void addToCommandHistory(const std::string &command);
    const std::vector<std::string>& getCommandHistory() const;
    
    // Command completion
    void setCompletions(const std::vector<std::string> &completions);
    std::string completeCommand(const std::string &partial);
    std::string completeCommandContextual(const std::string &command_line);
    
    // Display history management
    void addToDisplayHistory(const std::string &message);
    void clearDisplayHistory();
    void removeFromDisplayHistory(size_t index);
    size_t getDisplayHistorySize() const;
    const std::string& getDisplayHistoryAt(size_t index) const;
    const std::vector<std::string>& getDisplayHistory() const;

    // Interactive mode
    void runInteractive(std::function<bool(const std::string &)> commandHandler);
    void setPrompt(const std::string &prompt) { prompt_ = prompt; }

    // Command processing
    void setCommandHandler(std::function<bool(const std::string &)> handler) {
      commandHandler_ = handler;
    }
    
    // Connection status
    void setConnectionStatus(const std::string& status) {
      connectionStatus_ = status;
    }
    
    // Debug level
    void setDebugLevel(int level) {
      debugLevel_ = level;
    }
    
    int getDebugLevel() const {
      return debugLevel_;
    }
    
    bool isDestroying() const {
      return destroying_;
    }

    // Display methods (public for signal handler access)
    void putMessages();
    void putPrompt();
    void redrawScreen(); // Full screen redraw for signal handler
    void putFormattedText(const std::string &format, const std::string &text); // Write formatted text
    void clearToEndOfLine(); // Clear from cursor to end of line
    void handleResize(); // Handle window resize signal

    // Logger integration
    void setLoggerInstance(TUI* tui);
    void initializeLogger();
    void cleanupLogger();

    // Text formatting and wrapping
    std::vector<std::string> wrapText(const std::string &text, int width);
    std::vector<std::string> wrapTextToScreen(const std::string &text);
    std::vector<std::string> wrapTextWithIndent(const std::string &text, int width, int indent);
    std::string truncateText(const std::string &text, int width);
    std::vector<std::string> splitWords(const std::string &text);
    int getTextWidth(const std::string &text);
    void putWrappedText(const std::string &text);
    void putIndentedText(const std::string &text, int indent);

  private:
    bool initialized_;
    std::string prompt_;
    std::vector<std::string> commandHistory_;
    std::vector<std::string> displayHistory_;
    std::vector<std::string> completions_;
    std::function<bool(const std::string &)> commandHandler_;
    int commandHistoryPosition_;
    int scrollOffset_;
    std::string connectionStatus_;
    int debugLevel_;
    bool destroying_; // Flag to prevent logging during destruction
    std::unique_ptr<netd::client::netconf::NetconfClient> client_;

    // Curses helpers
    void setupCurses();
    void setupColors();
    void configureSignalHandler();
    void initializeScreen();
    void enableRawMode();
    void disableEcho();
    void enableKeypad();
    void cleanupScreen();
    int scanKeyInput();
    int getPromptLength();
    void moveCursor(int y, int x);
    void sleepMs(int ms);
    void resizeTerminal();
    void setAttribute(int attr);
    void addAttribute(int attr);
    void removeAttribute(int attr);
    void clearCurses();
    void refreshCurses();
    void doUpdateCurses();
    void putChar(char ch);
    std::string formatReturnValue(bool ctrl_d_exit, const std::string &result);
    void handleKeyInput(int key);
    void redrawPrompt();
    void scrollMessages();
    void scrollMessagesDown();
    
    // Status bar functions
    void putStatusBar();
    void updateStatusBar(const std::string& message);
    void clearStatusBar();
    std::string getScrollInfo();
    
    // Key input handlers
    void handleBackspace();
    void handleDelete();
    void handleLeftArrow();
    void handleRightArrow();
    void handleUpArrow();
    void handleDownArrow();
    void handlePageUp();
    void handlePageDown();
    void handlePrintableChar(int key);
    void handleCtrlD();
    void handleCtrlL();
    
    // Input state management
    void resetInputState();
    
    // Screen dimension helpers
    int getScreenSizeX();
    int getScreenSizeY();
    int getPromptRow();
    
    // Input helpers
    std::string scanPromptLine();
    void putCurrentLine(const std::string &line);
    void clearCurrentLine();
    void insertCharAtCursor(char ch);
    void deleteCharAtCursor();
    void backspaceAtCursor();
    int getCursorX();
    int getCursorY();
    int getMaxLines();
    int getScrollOffset();
    void setScrollOffset(int offset);
    
    // History position helpers
    int getHistoryPosition();
    void setHistoryPosition(int position);
    void advanceHistoryPosition();
  };

} // namespace netd::client::tui

#endif // NETD_CLIENT_TUI_HPP
