#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include <windows.h>

// Функция обратного вызова для записи данных
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::ofstream* out = static_cast<std::ofstream*>(userp);
    size_t totalSize = size * nmemb;
    out->write(static_cast<const char*>(contents), totalSize);
    return totalSize;
}

// Функция для загрузки файла
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

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    outFile.close();
    return true;
}

// Функция для проверки существования каталога
bool directoryExists(const std::string& dir) {
    DWORD ftyp = GetFileAttributesA(dir.c_str());
    return (ftyp != INVALID_FILE_ATTRIBUTES && (ftyp & FILE_ATTRIBUTE_DIRECTORY));
}

// Функция для создания каталога
void createDirectory(const std::string& dir) {
    CreateDirectoryA(dir.c_str(), nullptr);
}

int main() {
    std::string appDataPath = getenv("APPDATA") + std::string("\\ZcashParams");

    // Создание директории, если она не существует
    if (!directoryExists(appDataPath)) {
        createDirectory(appDataPath);
    }

    // Массив URL для загрузки
    std::string filesToDownload[] = {
        "https://komodoplatform.com/downloads/sprout-proving.key",
        "https://komodoplatform.com/downloads/sprout-verifying.key",
        "https://komodoplatform.com/downloads/sapling-spend.params",
        "https://komodoplatform.com/downloads/sapling-output.params",
        "https://komodoplatform.com/downloads/sprout-groth16.params"
    };

    for (const auto& fileUrl : filesToDownload) {
        std::string fileName = fileUrl.substr(fileUrl.find_last_of("/") + 1);
        std::string localPath = appDataPath + "\\" + fileName;

        // Проверка существования файла
        std::ifstream infile(localPath);
        if (!infile.good()) {
            std::cout << "Downloading " << fileName << ", this may take a while ..." << std::endl;
            if (downloadFile(fileUrl, localPath)) {
                std::cout << "Download complete: " << localPath << std::endl;
            }
        } else {
            std::cout << fileName << " already exists, skipping download." << std::endl;
        }
    }

    std::cout << "All files are downloaded or already exist." << std::endl;
    return 0;
}
