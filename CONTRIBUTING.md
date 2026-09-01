# Contributing to BBS Terminal Emulator

Thank you for your interest in contributing! This document provides guidelines and instructions for contributing.

## Code of Conduct

Be respectful and constructive in all interactions. This project welcomes contributors from all backgrounds.

## Getting Started

1. **Fork the repository** on GitHub
2. **Clone your fork:**
   ```bash
   git clone https://github.com/YOUR_USERNAME/bbs-terminal-emulator.git
   cd bbs-terminal-emulator
   ```
3. **Create a branch for your feature:**
   ```bash
   git checkout -b feature/your-feature-name
   ```

## Development Workflow

### Before Starting

- Check existing [Issues](https://github.com/jpbock/bbs-terminal-emulator/issues) to avoid duplicate work
- Open a new issue to discuss major changes before implementing
- Assign yourself if you plan to work on something

### Code Style

- **Language:** C++20
- **Namespaces:** Use to organize code (telnet, terminal, ui, dialing, config)
- **Naming:**
  - Classes: `PascalCase` (e.g., `TelnetClient`)
  - Functions/methods: `snake_case` (e.g., `process_input`)
  - Private members: `snake_case_` with trailing underscore
  - Constants: `UPPER_SNAKE_CASE`

- **Formatting:**
  - Indentation: 4 spaces
  - Line length: Keep under 100 characters where practical
  - Braces: Allman style for methods, inline for simple statements

### Example Code Style

```cpp
namespace terminal {

class ANSIInterpreter {
public:
    ANSIInterpreter(int width = 80, int height = 25);
    ~ANSIInterpreter();

    void process_input(const std::vector<unsigned char>& data);
    int get_cursor_x() const { return cursor_x_; }

private:
    void process_escape_sequence(const std::string& seq);

    int width_, height_;
    int cursor_x_ = 0, cursor_y_ = 0;
};

} // namespace terminal
```

### Commits

- Use clear, descriptive commit messages
- Start with a verb: "Add", "Fix", "Update", "Refactor", etc.
- Examples:
  - `Add NAWS telnet negotiation support`
  - `Fix ANSI color parsing for extended colors`
  - `Refactor screen buffer for better performance`

### Pull Requests

1. **Push to your fork:**
   ```bash
   git push origin feature/your-feature-name
   ```

2. **Create a Pull Request** on GitHub with:
   - Clear title describing the changes
   - Detailed description of what and why
   - Reference any related issues (e.g., "Fixes #123")
   - Screenshots for UI changes

3. **PR Guidelines:**
   - Keep PRs focused on a single feature or fix
   - Ensure code compiles without warnings
   - Add tests if applicable
   - Update documentation if needed
   - Be responsive to review feedback

## Testing

### Manual Testing Checklist

- [ ] Application launches without errors
- [ ] Telnet connection succeeds to a real BBS
- [ ] ANSI colors render correctly
- [ ] Text attributes (bold, underline, etc.) display
- [ ] Window resizing works
- [ ] Phonebook saves and loads
- [ ] Settings persist across sessions

### Performance Testing

- Test with large amounts of text data (1MB+)
- Monitor memory usage during extended sessions
- Check CPU usage with rapid screen updates

## Documentation

- Update README.md for new features
- Add code comments for complex logic
- Include examples for new functionality
- Update BUILDING.md if dependencies change

## Feature Ideas

We're interested in PRs for:

- **File Transfer:** Zmodem, Ymodem, Kermit protocols
- **Graphics:** Sixel support, better RIP rendering
- **Scripting:** Lua or Python support for macros
- **UI Improvements:** Theme customization, better phonebook UI
- **Performance:** Rendering optimizations, memory efficiency
- **Compatibility:** Better 8-bit character support, more terminal types

## Areas Needing Help

1. **Documentation** - README, tutorials, API docs
2. **Testing** - Test cases, bug reports
3. **Graphics** - RIP interpreter improvements
4. **UI Enhancements** - Modern Windows UI updates
5. **Performance** - Profiling and optimization

## Issue Labels

- `bug` - Something isn't working
- `enhancement` - New feature or improvement
- `documentation` - Docs or README updates
- `good-first-issue` - Good for newcomers
- `help-wanted` - Needs community assistance
- `in-progress` - Currently being worked on

## Questions?

- Check [GitHub Discussions](https://github.com/jpbock/bbs-terminal-emulator/discussions)
- Open an issue with `question` label
- Review existing documentation

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

## Recognition

Contributors will be recognized in:
- This CONTRIBUTING.md file
- GitHub contributors page
- Release notes for major contributions

Thank you for making BBS Terminal Emulator better!
