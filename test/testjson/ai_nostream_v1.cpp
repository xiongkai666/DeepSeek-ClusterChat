// #ifndef V1_CHATSERVICE_H
// #define V1_CHATSERVICE_H
#include <curl/curl.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "json.hpp"

using json = nlohmann::json;

// 回调函数，用于处理服务器响应
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    size_t real_size = size * nmemb;
    data->append(static_cast<char*>(contents), real_size);
    return real_size;
}

std::string filterThinkTags(const std::string& input) {
    std::string result = input;
    size_t start = 0;

    while ((start = result.find("</think>", start)) != std::string::npos) {
        size_t end = result.find("</think>", start);
        if (end != std::string::npos) {
            result.erase(start, end - start + 8);
        } else {
            result.erase(start);
            break;
        }
    }
    return result;
}

class AI {
   public:
    AI(const std::string& ainame) {
        // 初始化curl
        if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
            throw std::runtime_error("curl_global_init failed");
        }

        curl = curl_easy_init();
        if (!curl) {
            curl_global_cleanup();
            throw std::runtime_error("curl_easy_init failed");
        }

        // 配置API请求
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.deepseek.com/chat/completions");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);  // 设置超时时间30秒

        // 设置请求头
        headers = curl_slist_append(headers, "Content-Type: application/json");
        const char* apiKey = std::getenv("DEEPSEEK_API_KEY");
        if (!apiKey) {
            throw std::runtime_error("DEEPSEEK_API_KEY environment variable not set");
        }
        headers = curl_slist_append(headers, (std::string("Authorization: Bearer ") + apiKey).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // 初始化JSON数据
        jsData["model"] = ainame;
        jsData["stream"] = false;
    }

    std::string chat(const std::string& input) {
        std::string response;
        json userData = {
            {"role", "user"},
            {"content", input}};
        vecmsg.push_back(userData);
        jsData["messages"] = vecmsg;

        std::string jsonData = jsData.dump();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            return "服务器请求失败";
        }

        try {
            json jsResponse = json::parse(response);
            std::string content = filterThinkTags(jsResponse["choices"][0]["message"]["content"]);

            json assistantData = {
                {"role", "assistant"},
                {"content", content}};
            vecmsg.push_back(assistantData);
            return content;
        } catch (const json::parse_error& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
            return "服务器响应格式错误";
        }
    }

    void reset() {
        vecmsg.clear();
    }

    ~AI() {
        if (headers)
            curl_slist_free_all(headers);
        if (curl)
            curl_easy_cleanup(curl);
        curl_global_cleanup();
    }

   private:
    CURL* curl = nullptr;
    struct curl_slist* headers = nullptr;
    json jsData;
    std::vector<json> vecmsg;
};

int main() {
    try {
        AI ai("deepseek-reasoner");
        std::cout << "你好！我是Deepseek聊天机器人，输入exit退出" << std::endl;

        while (true) {
            std::string input;
            std::cout << "> ";
            std::getline(std::cin, input);

            if (input == "exit") {
                break;
            }

            std::string response = ai.chat(input);
            std::cout << "AI: " << response << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

// #endif