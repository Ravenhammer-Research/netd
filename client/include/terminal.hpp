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

#ifndef NETD_CLIENT_TERMINAL_HPP
#define NETD_CLIENT_TERMINAL_HPP

#include <functional>
#include <string>
#include <vector>

namespace netd::client {

  class Terminal {
  public:
    Terminal();
    ~Terminal();

    // Terminal control
    bool initialize();
    void cleanup();
    bool isInitialized() const { return initialized_; }

    // Input/Output
    std::string readLine();
    void write(const std::string &text);
    void writeLine(const std::string &text);
    void clear();
    void refresh();

    // Command history
    void addToHistory(const std::string &command);
    std::string getHistoryUp();
    std::string getHistoryDown();
    void resetHistoryPosition();

    // Command completion
    void setCompletions(const std::vector<std::string> &completions);
    std::string completeCommand(const std::string &partial);

    // Interactive mode
    void runInteractive();
    void setPrompt(const std::string &prompt) { prompt_ = prompt; }
    void redrawPrompt();

    // Command processing
    void setCommandHandler(std::function<bool(const std::string &)> handler) {
      commandHandler_ = handler;
    }

  private:
    bool initialized_;
    std::string prompt_;
    std::vector<std::string> history_;
    int historyPosition_;
    std::vector<std::string> completions_;
    std::function<bool(const std::string &)> commandHandler_;
  };

} // namespace netd::client

#endif // NETD_CLIENT_TERMINAL_HPP