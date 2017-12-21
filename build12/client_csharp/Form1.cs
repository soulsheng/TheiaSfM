using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using System.Runtime.InteropServices;

namespace client_csharp
{

    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        [DllImport(@"dll_reconstructiond.dll", EntryPoint = "kernelReBuildSparse", CallingConvention = CallingConvention.Cdecl)]
        public static extern int kernelReBuildSparse(string inputImageDir, StringBuilder resultString);

        private void button1_Click(object sender, EventArgs e)
        {
            string imagePath = "E:\\3d\\2017_03-5\\";
            StringBuilder retString = new StringBuilder();
            int ret = kernelReBuildSparse(imagePath, retString);
            label1.Text = ret.ToString() + retString;
        }
    }
}
