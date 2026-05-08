#include "dialogue_manager.hpp"

#include <nlohmann/json.hpp>

#include <sstream>

static std::string WrapTextToWidth(const std::string &text, Font font, float fontSize, float maxWidth)
{
    if (text.empty())
        return text;

    std::stringstream ss(text);
    std::string finalText;
    std::string currentLine;
    std::string word;

    while (ss >> word)
    {
        std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
        Vector2 textSize = MeasureTextEx(font, testLine.c_str(), fontSize, 1.0f);

        if (textSize.x <= maxWidth)
        {
            currentLine = testLine;
        }
        else
        {
            if (!currentLine.empty())
                finalText += currentLine + "\n";

            currentLine = word;
        }
    }

    if (!currentLine.empty())
        finalText += currentLine;

    return finalText;
}

static std::vector<std::string> SplitLines(const std::string &text)
{
    std::vector<std::string> lines;
    std::stringstream ss(text);
    std::string line;

    while (std::getline(ss, line, '\n'))
        lines.push_back(line);

    return lines;
}

DialogueManager::DialogueManager()
    : timer_{0.0f}, speed_{0.05f}, currentIndex_{0}, selectedChoiceIndex_{0},
      choicesVisible_{false}, isTypingFinished_{false} {}

void DialogueManager::setStyle(Font font, int screenWidth, int screenHeight,
                               float fontSize, Color mainColor,
                               Color outlineColor, float outlineWidth)
{
    font_ = font;
    screenWidth_ = screenWidth;
    screenHeight_ = screenHeight;
    mainFontSize_ = fontSize;
    mainColor_ = mainColor;
    outlineColor_ = outlineColor;
    outlineWidth_ = outlineWidth;
    hasStyle_ = true;
}

void DialogueManager::startConversation(const std::string &nodeID)
{
    auto it = currentConversation_.find(nodeID);
    if (it == currentConversation_.end())
    {
        clear();
        return;
    }
    currentNodeIndex_ = nodeID;
    targetText_ = it->second.text;
    visibleText_.clear();
    timer_ = 0.0f;
    currentIndex_ = 0;
    selectedChoiceIndex_ = 0;
    isTypingFinished_ = false;
    choicesVisible_ = false;
    wrappedChoices_.clear();

    if (!it->second.triggerEvent.empty())
        pendingEvent_ = it->second.triggerEvent;

    float mainTextWrapWidth = static_cast<float>(screenWidth_ - 200);
    float choiceWrapWidth = static_cast<float>(screenWidth_ - 140);

    if (mainTextWrapWidth < 120.0f)
        mainTextWrapWidth = 120.0f;
    if (choiceWrapWidth < 120.0f)
        choiceWrapWidth = 120.0f;

    targetText_ = WrapTextToWidth(targetText_, font_, mainFontSize_, mainTextWrapWidth);

    for (const auto &choice : it->second.choices)
    {
        std::string wrappedChoice = WrapTextToWidth(choice.text, font_, chooseFontSize_, choiceWrapWidth);
        wrappedChoices_.push_back(SplitLines(wrappedChoice));
    }
}

void DialogueManager::update(float dt, AudioManager &audio)
{
    if (targetText_.empty())
        return;

    if (isTypingFinished_ == false)
    {
        if (IsKeyPressed(KEY_E))
        {
            skip();
            return;
        }

        timer_ += dt;
        if ((timer_ >= speed_) && (currentIndex_ < targetText_.length()))
        {
            visibleText_ += targetText_[currentIndex_];
            currentIndex_++;
            timer_ -= speed_;
            audio.playSound("blip");
        }
        if (currentIndex_ >= targetText_.length())
        {
            isTypingFinished_ = true;
        }
    }
    else if (isTypingFinished_ == true)
    {
        auto it = currentConversation_.find(currentNodeIndex_);
        if (it == currentConversation_.end())
            return;

        int choiceSize = it->second.choices.size();
        if (choiceSize == 0 && it->second.nextLineId.empty())
        {
            if (IsKeyPressed(KEY_E))
            {
                clear();
            }
            return;
        }
        if (selectedChoiceIndex_ < 0)
            selectedChoiceIndex_ = 0;
        if (selectedChoiceIndex_ >= choiceSize)
            selectedChoiceIndex_ = choiceSize - 1;

        if (!it->second.nextLineId.empty() && IsKeyPressed(KEY_E))
        {
            startConversation(it->second.nextLineId);
            return;
        }

        if (choicesVisible_)
        {
            if (IsKeyPressed(KEY_W) && selectedChoiceIndex_ > 0)
                selectedChoiceIndex_--;

            if (IsKeyPressed(KEY_S) && (selectedChoiceIndex_ < choiceSize - 1))
                selectedChoiceIndex_++;

            if (IsKeyPressed(KEY_E))
            {
                if (!it->second.triggerEvent.empty())
                    pendingEvent_ = it->second.triggerEvent;
                startConversation(it->second.choices[selectedChoiceIndex_].nextNodeID);
                return;
            }
        }

        if (IsKeyPressed(KEY_E) && (!choicesVisible_))
        {
            choicesVisible_ = true;
        }
    }
}

std::string DialogueManager::popEvent()
{
    std::string eventToReturn = pendingEvent_;
    pendingEvent_.clear();
    return eventToReturn;
}

void DialogueManager::skip()
{
    visibleText_ = targetText_;
    currentIndex_ = targetText_.length();
    isTypingFinished_ = true;
}

void DialogueManager::render()
{
    std::vector<std::string> seglist = SplitLines(visibleText_);
    if (seglist.empty() && wrappedChoices_.empty())
        return;

    float lineSpacing = 60.0f;
    float choiceGap = 25.0f;
    float anchorY = screenHeight_ - 50.0f;
    float mainTextHeight = seglist.size() * lineSpacing;
    float chooseTextHeight = 0.0f;

    float maxWidth{0.0f};
    for (auto const &seg : seglist)
    {
        Vector2 textSize = MeasureTextEx(font_, seg.c_str(), mainFontSize_, 1.0f);
        if (textSize.x > maxWidth)
            maxWidth = textSize.x;
    }

    if (choicesVisible_)
    {
        for (const auto &wrappedChoice : wrappedChoices_)
            chooseTextHeight += static_cast<float>(wrappedChoice.size()) * lineSpacing;
    }

    float startY = (choicesVisible_) ? anchorY - (mainTextHeight + choiceGap + chooseTextHeight)
                                     : anchorY - mainTextHeight;

    float rectWidth = maxWidth + 10.0f;
    float rectHeight = mainTextHeight + 5.0f;
    float rectX = (screenWidth_ - rectWidth) / 2.0f;
    float rectY = (choicesVisible_) ? anchorY - (rectHeight + choiceGap + chooseTextHeight)
                                    : anchorY - rectHeight;
    DrawRectangle(rectX, rectY, rectWidth, rectHeight, Fade(BLACK, 0.3f));

    for (std::size_t i = 0; i < seglist.size(); i++)
    {
        const std::string &seg = seglist[i];
        Vector2 textSize = MeasureTextEx(font_, seg.c_str(), mainFontSize_, 1.0f);
        Vector2 basePosition = {(screenWidth_ - textSize.x) / 2.0f, startY + i * lineSpacing};

        DrawTextEx(font_, seg.c_str(),
                   {basePosition.x, basePosition.y - outlineWidth_}, mainFontSize_, 1.0f, outlineColor_);
        DrawTextEx(font_, seg.c_str(),
                   {basePosition.x, basePosition.y + outlineWidth_}, mainFontSize_, 1.0f, outlineColor_);
        DrawTextEx(font_, seg.c_str(),
                   {basePosition.x - outlineWidth_, basePosition.y}, mainFontSize_, 1.0f, outlineColor_);
        DrawTextEx(font_, seg.c_str(),
                   {basePosition.x + outlineWidth_, basePosition.y}, mainFontSize_, 1.0f, outlineColor_);
        DrawTextEx(font_, seg.c_str(),
                   {basePosition.x, basePosition.y}, mainFontSize_, 1.0f, mainColor_);
    }
    if (choicesVisible_ == true)
    {
        float choiceStartY = startY + mainTextHeight + choiceGap;
        float choiceYCursor = choiceStartY;

        for (int i = 0; i < static_cast<int>(wrappedChoices_.size()); i++)
        {
            Color selected = (i == selectedChoiceIndex_) ? YELLOW : WHITE;

            for (std::size_t lineIndex = 0; lineIndex < wrappedChoices_[i].size(); lineIndex++)
            {
                const std::string &choiceLine = wrappedChoices_[i][lineIndex];
                Vector2 textSize = MeasureTextEx(font_, choiceLine.c_str(), chooseFontSize_, 1.0f);
                Vector2 basePosition = {((screenWidth_ - textSize.x) / 2.0f), choiceYCursor + lineIndex * lineSpacing};

                DrawTextEx(font_, choiceLine.c_str(),
                           {basePosition.x, basePosition.y - outlineWidth_}, chooseFontSize_, 1.0f, outlineColor_);
                DrawTextEx(font_, choiceLine.c_str(),
                           {basePosition.x, basePosition.y + outlineWidth_}, chooseFontSize_, 1.0f, outlineColor_);
                DrawTextEx(font_, choiceLine.c_str(),
                           {basePosition.x - outlineWidth_, basePosition.y}, chooseFontSize_, 1.0f, outlineColor_);
                DrawTextEx(font_, choiceLine.c_str(),
                           {basePosition.x + outlineWidth_, basePosition.y}, chooseFontSize_, 1.0f, outlineColor_);
                DrawTextEx(font_, choiceLine.c_str(),
                           {basePosition.x, basePosition.y}, chooseFontSize_, 1.0f, selected);
            }

            choiceYCursor += static_cast<float>(wrappedChoices_[i].size()) * lineSpacing;
        }
    }
}

void DialogueManager::clear()
{
    targetText_.clear();
    visibleText_.clear();
    timer_ = 0.0f;
    currentIndex_ = 0;
    currentNodeIndex_.clear();
    selectedChoiceIndex_ = 0;
    isTypingFinished_ = false;
    choicesVisible_ = false;
    wrappedChoices_.clear();
}

void DialogueManager::loadFromJSON(const std::string &path)
{
    using json = nlohmann::json;
    currentConversation_.clear();

    char *dialogueText = LoadFileText(path.c_str());
    if (dialogueText == nullptr)
    {
        TraceLog(LOG_ERROR, "Dialogues file not found");
        return;
    }

    std::string text = dialogueText;
    UnloadFileText(dialogueText);

    try
    {
        json j = json::parse(text);

        for (const auto &[key, val] : j.items())
        {
            DialogueNode node;
            node.speakerName = val["speakerName"];
            node.text = val["text"];
            for (const auto &i : val["choices"])
            {
                DialogueChoice choice;
                choice.text = i["text"];
                choice.nextNodeID = i["nextNodeID"];
                node.choices.push_back(choice);
            }
            node.nextLineId = val["nextLineId"];
            if (val.contains("triggerEvent"))
                node.triggerEvent = val["triggerEvent"];
            currentConversation_[key] = node;
        }
    }
    catch (const std::exception &e)
    {
        TraceLog(LOG_ERROR, "JSON Parsing error: %s", e.what());
    }
}
