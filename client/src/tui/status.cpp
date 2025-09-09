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

#include <client/include/tui.hpp>
#include <curses.h>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace netd::client {

  // Get scroll information string
  std::string TUI::getScrollInfo() {
    if (displayHistory_.empty()) {
      return "";
    }
    
    int screenWidth = getScreenSizeX();
    int maxLines = getMaxLines();
    
    // Calculate total lines needed for all messages
    int totalLines = 0;
    for (const auto& message : displayHistory_) {
      std::vector<std::string> wrapped = wrapText(message, screenWidth);
      totalLines += static_cast<int>(wrapped.size());
    }
    
    // Only show page count if there's more than 1 page
    if (totalLines <= maxLines) {
      return "";
    }
    
    // Calculate current page and total pages
    int currentPage = (scrollOffset_ / maxLines) + 1;
    int totalPages = (totalLines + maxLines - 1) / maxLines; // Ceiling division
    
    std::ostringstream oss;
    oss << currentPage << "/" << totalPages;
    return oss.str();
  }

  // Status bar display
  void TUI::putStatusBar() {
    int screenWidth = getScreenSizeX();
    int screenHeight = getScreenSizeY();
    
    // Don't draw status bar if screen is too small
    if (screenHeight < 3) {
      return;
    }
    
    // Move to top line
    move(0, 0);
    
    // Clear the status bar line
    clrtoeol();
    
    // Left side: Connection status
    std::string leftText = connectionStatus_;
    
    // Right side: Scroll information
    std::string rightText = getScrollInfo();
    
    // Calculate positions
    int leftPos = 1;
    int rightPos = screenWidth - rightText.length() - 1;
    
    // Draw left text with bold only on hostname/filename
    if (leftText.find("Connected to ") == 0) {
      // Print "Connected to " in normal formatting
      std::string prefix = "Connected to ";
      mvprintw(0, leftPos, "%s", prefix.c_str());
      
      // Print the hostname/filename with bold
      std::string hostname = leftText.substr(prefix.length());
      attron(A_BOLD);
      mvprintw(0, leftPos + prefix.length(), "%s", hostname.c_str());
      attroff(A_BOLD);
    } else {
      // For other status messages (like "Disconnected"), print normally
      mvprintw(0, leftPos, "%s", leftText.c_str());
    }
    
    // Draw right text (only if there's scroll info to show)
    if (!rightText.empty()) {
      // Parse the page numbers and make digits bold
      size_t slashPos = rightText.find('/');
      if (slashPos != std::string::npos) {
        // Print first number in bold
        std::string firstNum = rightText.substr(0, slashPos);
        attron(A_BOLD);
        mvprintw(0, rightPos, "%s", firstNum.c_str());
        attroff(A_BOLD);
        
        // Print slash in normal
        mvprintw(0, rightPos + slashPos, "/");
        
        // Print second number in bold
        std::string secondNum = rightText.substr(slashPos + 1);
        attron(A_BOLD);
        mvprintw(0, rightPos + slashPos + 1, "%s", secondNum.c_str());
        attroff(A_BOLD);
      } else {
        // Fallback: print entire string in bold
        attron(A_BOLD);
        mvprintw(0, rightPos, "%s", rightText.c_str());
        attroff(A_BOLD);
      }
    }
  }

  // Update status bar with custom message
  void TUI::updateStatusBar(const std::string& message) {
    int screenWidth = getScreenSizeX();
    int screenHeight = getScreenSizeY();
    
    // Don't draw status bar if screen is too small
    if (screenHeight < 3) {
      return;
    }
    
    // Move to top line
    move(0, 0);
    
    // Set status bar attributes (reverse video)
    attron(A_REVERSE);
    
    // Clear the status bar line
    clrtoeol();
    
    // Center the message
    int messagePos = (screenWidth - message.length()) / 2;
    if (messagePos < 1) {
      messagePos = 1;
    }
    
    // Draw the message
    mvprintw(0, messagePos, "%s", message.c_str());
    
    // Turn off reverse video
    attroff(A_REVERSE);
  }

  // Clear status bar
  void TUI::clearStatusBar() {
    int screenHeight = getScreenSizeY();
    
    // Don't draw status bar if screen is too small
    if (screenHeight < 3) {
      return;
    }
    
    // Move to top line and clear it
    move(0, 0);
    clrtoeol();
  }

} // namespace netd::client
