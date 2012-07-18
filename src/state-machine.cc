
struct CStateMachine {
    bool in_backslash;
    bool in_comment;
    bool in_string;
    bool in_char;

    CStateMachine() {
        reset();
    }
    CStateMachine(const std::string &line) {
        reset();
        update(line);
    }
    bool in_something() const {
        return in_backslash || in_comment || in_string || in_char;
    }
    void reset() {
        in_backslash = false;
        in_comment = false;
        in_string = false;
        in_char = false;
    }
    void update(const std::string &line_) {
        in_backslash = false;
        const char *line = line_.c_str();
        for (int i=0; line[i] != '\0'; ++i) {
            if (line[i] == '\\' && line[i+1] == '\0') {
                in_backslash = true;
                return;
            } else if (in_string) {
                if (line[i] == '\\') ++i;
                else if (line[i] == '"') in_string = false;
            } else if (in_char) {
                if (line[i] == '\\') ++i;
                else if (line[i] == '\'') in_char = false;
            } else if (in_comment) {
                if (line[i] == '*' && line[i+1] == '/') {
                    ++i;
                    in_comment = false;
                }
            } else {
                if (line[i] == '/' && line[i+1] == '*') {
                    ++i;
                    in_comment = true;
                } else if (line[i] == '/' && line[i+1] == '/') {
                    const char *end = strchr(line, '\0');
                    if (end != line && end[-1] == '\\')
                        in_backslash = true;
                    return;
                } else if (line[i] == '"') {
                    in_string = true;
                } else if (line[i] == '\'') {
                    in_char = true;
                }
            }
        }
    }
};
