#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::ofstream* out = static_cast<std::ofstream*>(userp);
    size_t totalSize = size * nmemb;
    out->write(static_cast<const char*>(contents), totalSize);
    return totalSize;
}

bool downloadFile(const std::string& url, const std::string& outputPath) {
    CURL* curl;
    CURLcode res;
    std::ofstream outFile(outputPath, std::ios::binary);

    if (!outFile) {
        std::cerr << "Could not open file for writing: " << outputPath << std::endl;
        return false;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outFile);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    outFile.close();
    return res == CURLE_OK;
}

int main() {
    fs::path appDataPath;
    
    #ifdef _WIN32
        const char* appData = std::getenv("APPDATA");
        if (!appData) {
            std::cerr << "APPDATA environment variable not found!" << std::endl;
            return 1;
        }
        appDataPath = fs::path(appData) / "ZcashParams";
    #else
        const char* homeDir = std::getenv("HOME");
        if (!homeDir) {
            std::cerr << "HOME environment variable not found!" << std::endl;
            return 1;
        }
        appDataPath = fs::path(homeDir) / ".zcash-params";
    #endif

    try {
        fs::create_directories(appDataPath);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error creating directory: " << e.what() << std::endl;
        return 1;
    }

    std::string filesToDownload[] = {
        "https://komodoplatform.com/downloads/sprout-proving.key",
        "https://komodoplatform.com/downloads/sprout-verifying.key",
        "https://komodoplatform.com/downloads/sapling-spend.params",
        "https://komodoplatform.com/downloads/sapling-output.params",
        "https://komodoplatform.com/downloads/sprout-groth16.params"
    };

    for (const auto& fileUrl : filesToDownload) {
        std::string fileName = fileUrl.substr(fileUrl.find_last_of("/") + 1);
        fs::path localPath = appDataPath / fileName;

        if (!fs::exists(localPath)) {
            std::cout << "Downloading " << fileName << "...\n";
            if (downloadFile(fileUrl, localPath.string())) {
                std::cout << "Download complete: " << localPath << std::endl;
            } else {
                std::cerr << "Failed to download: " << fileName << std::endl;
            }
        } else {
            std::cout << fileName << " already exists, skipping download." << std::endl;
        }
    }

    std::cout << "All files processed." << std::endl;
    return 0;
}
