#ifndef ARGUMENT_PARSER_H_
#define ARGUMENT_PARSER_H_

#include <string>
#include <vector>

/**
 * Argument parser based on the unix getopt() and getopt_long() functions.
 * 
 * Construct the parser with an options list of the accepted arguments, call
 * setArguments() to initialize, then get options from nextOption() in a loop.
 * Options are considered as each character following a single dash '-' and
 * long options are identifiers following two dashes. If an option includes a
 * parameter passed to it, it must follow immediately after the option. After
 * all arguments finish parsing, nextOption() returns -1 and the contents of
 * argv are rearranged to place all non-option arguments (except for option
 * parameters) at the end. This uses a stable sort to preserve ordering. After
 * the rearrangement, getIndex() will return the index in argv of the first of
 * these non-option arguments.
 */
class ArgumentParser {
    public:
    enum ArgumentType {
        NoArg, RequiredArg, OptionalArg
    };
    
    struct OptionEntry {
        char shortName;
        const char* longName;
        ArgumentType argumentType;
        int* flagPtr;
        int value;
    };
    typedef std::vector<OptionEntry> OptionList;
    
    /**
     * Constructs the argument parser with the given options. For each option
     * at least one shortName or longName (corresponding to short and long
     * option) should be provided. If an option does not have one of these, a
     * null character '\0' or empty string should be provided respectively. The
     * argument type specifies if a parameter should be included after the
     * option. If the flagPtr is set, calling nextOption() on this option will
     * set the pointed-to integer to value and nextOption() will return 0.
     * Otherwise if flagPtr is nullptr, calling nextOption() will just return
     * value instead.
     */
    ArgumentParser(OptionList options);
    
    /**
     * Returns the parameter passed with the last call to nextOption(), or
     * nullptr if there was none.
     */
    const char* getOptionArg() const;
    
    /**
     * Returns the current index in argv of the next argument to parse. When
     * parsing finishes this is set to the index of the first non-option.
     */
    int getIndex() const;
    
    /**
     * Sets new argument list, the last argument in the list must be nullptr.
     * The startIndex specifies where to start parsing.
     * 
     * Example format of argv:
     * -abc --long-option "Long option parameter" -s short_option_parameter
     * argv = {"-abc", "--long-option", "\"Long option parameter\"", "-s",
     *     "short_option_parameter", nullptr}
     * In this case, argv does not start with the program name, so startIndex
     * would need to be set to zero.
     */
    void setArguments(const char** argv, int startIndex = 1);
    
    /**
     * Get the next option from the arguments list, returns -1 if no more
     * found. If the argument is not in the OptionList, returns '?'. If the
     * argument requires a parameter but none is provided, returns ':'. Both
     * errors will write an error message to errorMessagePtr if the string
     * pointer is provided. See constructor doc string for more details.
     */
    int nextOption(std::string* errorMessagePtr = nullptr);
    
    private:
    OptionList options_;
    const char** argv_;
    std::vector<const char*> nonOptionArguments_;
    const char* optionArg_;
    int index_;
    int charIndex_;
    
    /**
     * Increments charIndex_ if the current argument contains more short
     * options. If it doesn't, resets charIndex_ to zero and increments index_.
     */
    void nextShortOption();
    
    /**
     * Checks if s is an option. Returns 2 if long option, 1 if short, or 0 if
     * not an option.
     */
    int hasOptionFormat(const char* s);
    
    /**
     * Checks if the current index_ points to an argument.
     */
    bool hasParameter();
    
    /**
     * Returns the result of the option for the current index_ entry.
     */
    int foundOption(const OptionEntry& option, const std::string& optionStr);
};

#endif
