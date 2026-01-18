using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BrightData_WPF
{
    public class RepositoryData
    {
        public string Rank { get; set; } = "";    // 順位
        public string Name { get; set; } = "";   // リポジトリ名
        public string Stars { get; set; } = "";  // スター数
        public string Description { get; set; } = ""; // 説明
        public String Language { get; set; } = ""; // 使用言語
    }
}
