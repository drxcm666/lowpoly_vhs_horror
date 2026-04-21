#pragma once

#include "audio.hpp"

#include <raylib.h>

#include <string>
#include <vector>
#include <unordered_map>

struct DialogueChoice
{
    std::string text;
    std::string nextNodeID;
};

struct DialogueNode
{
    std::string speakerName;
    std::string text;
    std::vector<DialogueChoice> choices;
    std::string nextLineId;
};

class DialogueManager
{
private:
    std::string targetText_;
    std::string visibleText_;
    float timer_;
    float speed_;
    std::size_t currentIndex_;
    std::unordered_map<std::string, DialogueNode> currentConversation_;
    std::string currentNodeIndex_;
    int selectedChoiceIndex_;
    bool isTypingFinished_;
    bool choicesVisible_;

    Font font_{};
    int screenWidth_{0};
    int screenHeight_{0};
    float mainFontSize_{0};
    float chooseFontSize_{25.0f};
    Color mainColor_{WHITE};
    Color outlineColor_{BLACK};
    float outlineWidth_{2.0f};
    bool hasStyle_{false};
    std::vector<std::vector<std::string>> wrappedChoices_;

public:
    DialogueManager();
    void setStyle(Font font, int screenWidth, int screenHeight, float fontSize,
                  Color mainColor, Color outlineColor, float outlineWidth);
    void startConversation(const std::string &nodeID);
    void update(float dt, AudioManager &audio);
    void skip();
    void render();
    void clear();
    void loadDialogue();

    const std::string &getTargetText() const { return targetText_; };
    const std::string &getNodeText(const std::string &text)
    {
        return currentConversation_[text].text;
    };
};
