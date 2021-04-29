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
    } else if (charIndex_ == 0) {    // Check if the current argument has not been parsed yet.
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
                charIndex_ = 1;
                for (const OptionEntry& option : options_) {
                    if (option.shortName == argv_[index_][charIndex_]) {
                        nextShortOption();
                        return foundOption(option);
                    }
                }
                nextShortOption();    // Option not found.
                return '?';
            } else {    // Else, this argument is not an option.
                nonOptionArguments_.push_back(argv_[index_]);
                ++index_;
            }
        }
        
        // Done parsing, shuffle argv_ to put all of nonOptionArguments_ at the end.
        if (static_cast<int>(nonOptionArguments_.size()) == index_) {    // If all arguments are non-options, no shuffling required.
            index_ = 0;
            nonOptionArguments_.clear();
        } else if (!nonOptionArguments_.empty()) {    // Need to shuffle if at least one non-option found, otherwise the index_ is already good.
            int shiftDistance = 0;    // Number of places to shift the current argument left in the argv_ array.
            int i = 0;
            while (shiftDistance < static_cast<int>(nonOptionArguments_.size())) {    // Shift over elements not in the nonOptionArguments_ vector.
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
            
            index_ = i - shiftDistance;    // Update index_ and place all nonOptionArguments_ at the end.
            for (int j = 0; j < static_cast<int>(nonOptionArguments_.size()); ++j) {
                argv_[index_ + j] = nonOptionArguments_[j];
            }
        }
        
        return -1;
    } else {    // Else, continue parsing the argument.
        for (const OptionEntry& option : options_) {
            if (option.shortName == argv_[index_][charIndex_]) {
                nextShortOption();
                return foundOption(option);
            }
        }
        nextShortOption();    // Option not found.
        return '?';
    }
}

void ArgumentParser::nextShortOption() {
    ++charIndex_;
    if (argv_[index_][charIndex_] == '\0') {
        ++index_;
        charIndex_ = 0;
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
        ++index_;    // Note: We will never actually reach here because nextShortOption() is always called before hasParameter().
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
    
    if (option.flagPtr == nullptr) {
        return option.value;
    } else {
        *option.flagPtr = option.value;
        return 0;
    }
}
