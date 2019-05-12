#include "stdafx.h"
#include "RevConsole.h"
#include "RevEngineMain.h"
#include "imgui/imgui.h"

// Demonstrating creating a simple console window, with scrolling, filtering, completion and history.
// For the console example, here we are using a more C++ like approach of declaring a class to hold the data and the functions.
struct RevEngineConsole
{
	char                  m_inputBuffer[256];
	ImVector<char*>       History;
	ImVector<const char*> m_commands;
	ImVector<char*>       m_items;
	int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
	bool                  m_scrollToBottom;

	RevEngineConsole()
	{
		ClearLog();
		memset(m_inputBuffer, 0, sizeof(m_inputBuffer));
		HistoryPos = -1;
		m_commands.push_back("HELP");
		m_commands.push_back("HISTORY");
		m_commands.push_back("CLEAR");
		m_commands.push_back("CLASSIFY");
		m_commands.push_back("OPEN");// "classify" is here to provide an example of "C"+[tab] completing to "CL" and displaying matches.
	}

	// Portable helpers
	static int   StricmpStart(const char* str1, const char* str2);

	static int   Stricmp(const char* str1, const char* str2) { int d; while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; } return d; }
	static int   Strnicmp(const char* str1, const char* str2, int n) { int d = 0; while (n > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; n--; } return d; }
	static char* Strdup(const char *str) { size_t len = strlen(str) + 1; void* buff = malloc(len); return (char*)memcpy(buff, (const void*)str, len); }

	void ClearLog()
	{
		for (int i = 0; i < m_items.Size; i++)
			free(m_items[i]);
		m_items.clear();
		m_scrollToBottom = true;
	}

	void AddLog(const char* fmt, ...) IM_FMTARGS(2)
	{
		// FIXME-OPT
		char buf[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
		buf[IM_ARRAYSIZE(buf) - 1] = 0;
		va_end(args);
		m_items.push_back(Strdup(buf));
		m_scrollToBottom = true;
	}

	void Draw(const char* title)
	{
		ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin(title))
		{
			ImGui::End();
			return;
		}

		if (ImGui::SmallButton("Scroll to bottom")) m_scrollToBottom = true;

		ImGui::Separator();

		const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing(); // 1 separator, 1 input text
		ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), true, ImGuiWindowFlags_HorizontalScrollbar); // Leave room for 1 separator + 1 InputText
		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::Selectable("Clear")) ClearLog();
			ImGui::EndPopup();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
		for (int i = 0; i < m_items.Size; i++)
		{
			const char* item = m_items[i];
			ImVec4 col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // A better implementation may store a type per-item. For the sample let's just parse the text.
			if (strstr(item, "[error]")) col = ImColor(1.0f, 0.4f, 0.4f, 1.0f);
			else if (strncmp(item, "# ", 2) == 0) col = ImColor(1.0f, 0.78f, 0.58f, 1.0f);
			ImGui::PushStyleColor(ImGuiCol_Text, col);
			ImGui::TextUnformatted(item);
			ImGui::PopStyleColor();
		}

		if (m_scrollToBottom)
		{
			ImGui::SetScrollHere();
		}

		m_scrollToBottom = false;
		ImGui::PopStyleVar();
		ImGui::EndChild();
		ImGui::Separator();

		// Command-line
		if (ImGui::InputText("Input", m_inputBuffer, IM_ARRAYSIZE(m_inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackCharFilter, &TextEditCallbackStub, (void*)this))
		{
			char* input_end = m_inputBuffer + strlen(m_inputBuffer);
			while (input_end > m_inputBuffer && input_end[-1] == ' ') { input_end--; } *input_end = 0;
			if (m_inputBuffer[0])
				ExecCommand(m_inputBuffer);
			strcpy(m_inputBuffer, "");
		}

		// Demonstrate keeping auto focus on the input box
		if (ImGui::IsItemHovered() || (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
		{
			ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
		}
			
		ImGui::End();
	}

	void ExecCommand(const char* command_line)
	{
		AddLog("# %s\n", command_line);

		// Insert into history. First find match and delete it so it can be pushed to the back. This isn't trying to be smart or optimal.
		HistoryPos = -1;
		for (int i = History.Size - 1; i >= 0; i--)
			if (Stricmp(History[i], command_line) == 0)
			{
				free(History[i]);
				History.erase(History.begin() + i);
				break;
			}
		History.push_back(Strdup(command_line));

		// Process command
		if (Stricmp(command_line, "CLEAR") == 0)
		{
			ClearLog();
		}
		else if (Stricmp(command_line, "HELP") == 0)
		{
			AddLog("Commands:");
			for (int i = 0; i < m_commands.Size; i++)
				AddLog("- %s", m_commands[i]);
		}
		else if (Stricmp(command_line, "HISTORY") == 0)
		{
			int first = History.Size - 10;
			for (int i = first > 0 ? first : 0; i < History.Size; i++)
				AddLog("%3d: %s\n", i, History[i]);
		}
		else if (StricmpStart(command_line, "OPEN") == 0)
		{
			char buffer[512];
			strcpy(&buffer[0], &command_line[5]);
			AddLog("opening world: %s", buffer);
			RevEngineMain::LoadWorld(buffer);
		}
		else if (StricmpStart(command_line, "SPEED") == 0)
		{
			char buffer[512];
			strcpy(&buffer[0], &command_line[6]);
			extern float GMovementSpeed;
			GMovementSpeed = (float)atof(buffer);
		}

		else
		{
			AddLog("Unknown command: '%s'\n", command_line);
		}
	}

	static int TextEditCallbackStub(ImGuiTextEditCallbackData* data) // In C++11 you are better off using lambdas for this sort of forwarding callbacks
	{
		RevEngineConsole* console = (RevEngineConsole*)data->UserData;
		return console->TextEditCallback(data);
	}

	int TextEditCallback(ImGuiTextEditCallbackData* data)
	{
		//AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
		switch (data->EventFlag)
		{
		case ImGuiInputTextFlags_CallbackCompletion:
		{
			// Example of TEXT COMPLETION

			// Locate beginning of current word
			const char* word_end = data->Buf + data->CursorPos;
			const char* word_start = word_end;
			while (word_start > data->Buf)
			{
				const char c = word_start[-1];
				if (c == ' ' || c == '\t' || c == ',' || c == ';')
					break;
				word_start--;
			}

			// Build a list of candidates
			ImVector<const char*> candidates;
			for (int i = 0; i < m_commands.Size; i++)
				if (Strnicmp(m_commands[i], word_start, (int)(word_end - word_start)) == 0)
					candidates.push_back(m_commands[i]);

			if (candidates.Size == 0)
			{
				// No match
				AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
			}
			else if (candidates.Size == 1)
			{
				// Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
				data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
				data->InsertChars(data->CursorPos, candidates[0]);
				data->InsertChars(data->CursorPos, " ");
			}
			else
			{
				// Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and display "CLEAR" and "CLASSIFY"
				int match_len = (int)(word_end - word_start);
				for (;;)
				{
					int c = 0;
					bool all_candidates_matches = true;
					for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
						if (i == 0)
							c = toupper(candidates[i][match_len]);
						else if (c == 0 || c != toupper(candidates[i][match_len]))
							all_candidates_matches = false;
					if (!all_candidates_matches)
						break;
					match_len++;
				}

				if (match_len > 0)
				{
					data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
					data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
				}

				// List matches
				AddLog("Possible matches:\n");
				for (int i = 0; i < candidates.Size; i++)
					AddLog("- %s\n", candidates[i]);
			}

			break;
		}
		case ImGuiInputTextFlags_CallbackHistory:
		{
			// Example of HISTORY
			const int prev_history_pos = HistoryPos;
			if (data->EventKey == ImGuiKey_UpArrow)
			{
				if (HistoryPos == -1)
					HistoryPos = History.Size - 1;
				else if (HistoryPos > 0)
					HistoryPos--;
			}
			else if (data->EventKey == ImGuiKey_DownArrow)
			{
				if (HistoryPos != -1)
					if (++HistoryPos >= History.Size)
						HistoryPos = -1;
			}

			// A better implementation would preserve the data on the current input line along with cursor position.
			if (prev_history_pos != HistoryPos)
			{
				data->CursorPos = data->SelectionStart = data->SelectionEnd = data->BufTextLen = (int)snprintf(data->Buf, (size_t)data->BufSize, "%s", (HistoryPos >= 0) ? History[HistoryPos] : "");
				data->BufDirty = true;
			}
		}
		case ImGuiInputTextFlags_CallbackCharFilter:
		{
			if (data->EventChar == '`')
			{
				return 1;
			}
			return 0;
		}
		return 0;
		}
		return 0;
	}
};

int RevEngineConsole::StricmpStart(const char* str1, const char* str2)
{
	int d; 
	unsigned int index = 0;  
	unsigned int limit = (UINT)strlen(str2) ; 
	while (index < limit && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1)
	{ 
		str1++; 
		str2++; 
		index++; 
	} 
	return d; 
}


void RevConsole::Update()
{
	if (m_visible)
	{
		static RevEngineConsole console;
		console.Draw("Console");
	}
}


