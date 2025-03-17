using System;
using System.IO;
using System.Net;
using System.Threading.Tasks;

namespace ZcashDownloader
{
    class Program
    {
        static async Task Main(string[] args)
        {
            string appDataPath = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData);
            string directoryPath = Path.Combine(appDataPath, "ZcashParams");

            if (!Directory.Exists(directoryPath))
            {
                Directory.CreateDirectory(directoryPath);
            }

            var filesToDownload = new[]
            {
                "https://komodoplatform.com/downloads/sprout-proving.key",
                "https://komodoplatform.com/downloads/sprout-verifying.key",
                "https://komodoplatform.com/downloads/sapling-spend.params",
                "https://komodoplatform.com/downloads/sapling-output.params",
                "https://komodoplatform.com/downloads/sprout-groth16.params"
            };

            foreach (var fileUrl in filesToDownload)
            {
                string fileName = Path.GetFileName(fileUrl);
                string localPath = Path.Combine(directoryPath, fileName);

                if (!File.Exists(localPath))
                {
                    Console.WriteLine($"Downloading {fileName}, this may take a while ...");
                    await DownloadFileAsync(fileUrl, localPath);
                }
                else
                {
                    Console.WriteLine($"{fileName} already exists, skipping download.");
                }
            }

            Console.WriteLine("All files are downloaded or already exist.");
        }

        private static async Task DownloadFileAsync(string fileUrl, string localPath)
        {
            using (WebClient client = new WebClient())
            {
                await client.DownloadFileTaskAsync(new Uri(fileUrl), localPath);
            }
        }
    }
}
