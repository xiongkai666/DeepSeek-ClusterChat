// #ifndef CHATSERVICE_H
// #define CHATSERVICE_H

#include <iostream>
#include <stdexcept>
#include "ai.hpp"

size_t AI::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    size_t real_size = size * nmemb;
    data->append(static_cast<char*>(contents), real_size);
    return real_size;
}

AI::AI()
    : curl(nullptr), headers(nullptr) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.deepseek.com/chat/completions");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    // 设置请求头
    headers = curl_slist_append(headers, "Content-Type: application/json");
    const char* apiKey = std::getenv("DEEPSEEK_API_KEY");
    if (!apiKey) {
        curl_easy_cleanup(curl);
        throw std::runtime_error("DEEPSEEK_API_KEY environment variable not set");
    }
    headers = curl_slist_append(headers, (std::string("Authorization: Bearer ") + apiKey).c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    jsData["model"] = "deepseek-reasoner";
    jsData["stream"] = false;
}

AI::~AI() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
    if (headers) {
        curl_slist_free_all(headers);
    }
    curl_global_cleanup();
}

std::string AI::chat(std::string input) {
    std::string response;
    json userData;
    userData["role"] = "user";
    userData["content"] = input;
    vecmsg.push_back(userData);

    jsData["messages"] = vecmsg;
    std::string jsonData = jsData.dump();

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    std::string responseContext;

    if (res != CURLE_OK) {
        responseContext = "服务器请求失败";
    } else {
        try {
            json jsResponse = json::parse(response);
            if (jsResponse.contains("choices") && !jsResponse["choices"].empty()) {
                responseContext = filterThinkTags(jsResponse["choices"][0]["message"]["content"]);
            } else {
                responseContext = "无效的API响应";
            }
        } catch (const json::exception& e) {
            responseContext = "响应解析失败";
        }
    }

    json assistantData;
    assistantData["role"] = "assistant";
    assistantData["content"] = responseContext;
    vecmsg.push_back(assistantData);

    // 限制历史记录长度
    const size_t MAX_HISTORY = 5;
    if (vecmsg.size() > MAX_HISTORY * 2) {
        vecmsg.erase(vecmsg.begin(), vecmsg.begin() + 2);
    }

    return responseContext;
}

int AI::get_message_size() {
    return vecmsg.size();
}
size_t AI::WriteCallback(void* contents, size_t size, size_t nmemb, string* data) {
    size_t real_size = size * nmemb;
    data->append((char*)contents, real_size);
    return real_size;
}
string AI::filterThinkTags(const string& input) {
    string result = input;
    size_t start = 0;

    // 查找所有 <think> 标签
    while ((start = result.find("<think>", start)) != string::npos) {
        size_t end = result.find("</think>", start);
        if (end != string::npos) {
            // 删除 <think> 和 </think> 之间的内容
            result.erase(start, end - start + 8);  // 8 是 </think> 的长度
        } else {
            // 如果没有找到 </think>，删除从 <think> 开始的剩余部分
            result.erase(start);
            break;
        }
    }

    return result;
}