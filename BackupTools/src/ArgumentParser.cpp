#include "ArgumentParser.h"
#include <cstring>

ArgumentParser::ArgumentParser(OptionList options) :
    options_(options),
    argv_(nullptr),
    optionArg_(nullptr) {
}

const char* ArgumentParser::getOptionArg() const {
    return optionArg_;
}

int ArgumentParser::getIndex() const {
    return index_;
}

void ArgumentParser::setArguments(const char** argv, int startIndex) {
    argv_ = argv;
    nonOptionArguments_.clear();
    optionArg_ = nullptr;
    index_ = startIndex;
    charIndex_ = 0;
}

// ####################################################### TODO: finish errorMessagePtr

int ArgumentParser::nextOption(std::string* errorMessagePtr) {
    optionArg_ = nullptr;
    if (argv_ == nullptr) {
        return -1;
    } else if (charIndex_ == 0 || argv_[index_][charIndex_] == '\0') {    // Check if the current argument has not been parsed yet, or if no more characters left in the current one.
        if (charIndex_ != 0) {
            ++index_;
            charIndex_ = 0;
        }
        while (true) {
            if (argv_[index_] == nullptr) {    // Check if no more arguments.
                break;
            }
            
            int optionFormatResult = hasOptionFormat(argv_[index_]);
            if (optionFormatResult == 2) {    // Check long options.
                for (const OptionEntry& option : options_) {
                    if (std::strcmp(option.longName, argv_[index_] + 2) == 0) {
                        ++index_;
                        return foundOption(option);
                    }
                }
                ++index_;    // Option not found.
                return '?';
            } else if (optionFormatResult == 1) {    // Check short options.
                for (const OptionEntry& option : options_) {
                    if (option.shortName == argv_[index_][1]) {
                        charIndex_ = 2;
                        return foundOption(option);
                    }
                }
                charIndex_ = 2;    // Option not found.
                return '?';
            } else {    // Else, this argument is not an option.
                nonOptionArguments_.push_back(argv_[index_]);
                ++index_;
            }
        }
        
        // shuffle argv_ with nonOptionArguments_.
        
        if (static_cast<int>(nonOptionArguments_.size()) == index_) {    // If all arguments are non-options, no shuffling required.
            index_ = 0;
            nonOptionArguments_.clear();
        } else if (!nonOptionArguments_.empty()) {
            int shiftDistance = 0;
            int i = 0;
            while (shiftDistance < static_cast<int>(nonOptionArguments_.size())) {
                if (argv_[i] == nonOptionArguments_[shiftDistance]) {
                    ++shiftDistance;
                } else {
                    argv_[i - shiftDistance] = argv_[i];
                }
                ++i;
            }
            while (argv_[i] != nullptr) {
                argv_[i - shiftDistance] = argv_[i];
                ++i;
            }
            
            index_ = i - shiftDistance;
            for (int j = 0; j < static_cast<int>(nonOptionArguments_.size()); ++j) {
                argv_[index_ + j] = nonOptionArguments_[j];
            }
        }
        
        /*
        nonOptionArguments_:
        abc beans cool
        
        0   1      2     3    4     5
        abc --help beans cool --nou nouarg nullptr
        
        0      1     2      3   4     5
        --help --nou nouarg abc beans cool nullptr
        
        test1:
        shiftDistance = 0
        i = 0
        abc --help beans cool --nou nouarg nullptr
        
        shiftDistance = 1
        i = 1
        abc --help beans cool --nou nouarg nullptr
        
        shiftDistance = 1
        i = 2
        --help --help beans cool --nou nouarg nullptr
        
        shiftDistance = 2
        i = 3
        --help --help beans cool --nou nouarg nullptr
        
        shiftDistance = 3
        i = 4
        --help --help beans cool --nou nouarg nullptr
        
        shiftDistance = 3
        i = 5
        --help --nou beans cool --nou nouarg nullptr
        
        shiftDistance = 3
        i = 6
        --help --nou nouarg cool --nou nouarg nullptr
        */
        
        return -1;
    } else {    // Else, continue parsing the argument.
        for (const OptionEntry& option : options_) {
            if (option.shortName == argv_[index_][charIndex_]) {
                ++charIndex_;
                return foundOption(option);
            }
        }
        ++charIndex_;    // Option not found.
        return '?';
    }
}

int ArgumentParser::hasOptionFormat(const char* s) {
    if (s[0] == '-' && s[1] == '-' && s[2] != '\0') {
        return 2;
    } else if (s[0] == '-' && s[1] != '-' && s[1] != '\0') {
        return 1;
    } else {
        return 0;
    }
}

bool ArgumentParser::hasParameter() {
    if (charIndex_ != 0) {    // Check if short option.
        if (argv_[index_][charIndex_] != '\0') {    // If not at the end, no parameter.
            return false;
        }
        ++index_;
        charIndex_ = 0;
    }
    
    return argv_[index_] != nullptr && hasOptionFormat(argv_[index_]) == 0;
}

int ArgumentParser::foundOption(const OptionEntry& option) {
    if (option.argumentType == RequiredArg || option.argumentType == OptionalArg) {
        if (hasParameter()) {
            optionArg_ = argv_[index_];
            ++index_;
        } else if (option.argumentType == RequiredArg) {
            return ':';
        }
    }
    
    /*if (charIndex_ == 0) {    // Check if last was a long option.
        if (argv_[index_] != nullptr && hasOptionFormat(argv_[index_]) == 0) {
            // yay
        } else {
            return ':';
        }
    } else if (argv_[index_][charIndex_] == '\0') {    // Else, check if short option and it's at the end.
        ++index_;
        charIndex_ = 0;
        if (argv_[index_] != nullptr && hasOptionFormat(argv_[index_]) == 0) {
            // yay
        } else {
            return ':';
        }
    } else {
        return ':';
    }*/
    
    // may want to put all of the above in a function, might be tricky to update index_ and charIndex_ only when a parameter is needed tho (or maybe not idk)
    
    if (option.flagPtr == nullptr) {
        return option.value;
    } else {
        *option.flagPtr = option.value;
        return 0;
    }
}
