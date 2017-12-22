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

        string imagePath = "E:\\3d\\2017_03-5\\";
        string imagePathOutput = "E:\\3d\\output\\";
        StringBuilder filenameSparse = new StringBuilder();
        StringBuilder filenameDense = new StringBuilder();
        bool isLogInitialized = false;
        
        public Form1()
        {
            InitializeComponent();
            textBox1.Text = imagePath;
        }

        [DllImport(@"dll_reconstruction.dll", EntryPoint = "kernelReBuildSparse", CallingConvention = CallingConvention.Cdecl)]
        public static extern int kernelReBuildSparse(string inputImageDir, StringBuilder resultString);

        private void button1_Click(object sender, EventArgs e)
        {
            imagePath = textBox1.Text;
            int ret = kernelReBuildSparse(imagePath, filenameSparse);
            isLogInitialized = true;
            label1.Text = "return code:" + ret.ToString() + ", file:" + filenameSparse;
        }

        [DllImport(@"dll_reconstruction.dll", EntryPoint = "kernelReBuildDense", CallingConvention = CallingConvention.Cdecl)]
        public static extern int kernelReBuildDense(string inputImageDir, string filenameSparse, StringBuilder resultString, bool isLogInitialized);

        private void button2_Click(object sender, EventArgs e)
        {
            if(filenameSparse.Length == 0)
                filenameSparse.Append("E:\\3d\\2017_03-5\\result");
            int ret = kernelReBuildDense(imagePath, filenameSparse.ToString(), filenameDense, isLogInitialized);
            label2.Text = "return code:" + ret.ToString() + ", file:" + filenameDense;
        }

        [DllImport(@"dll_view3d.dll", EntryPoint = "render3DResult", CallingConvention = CallingConvention.Cdecl)]
        public static extern int render3DResult(string inputImageDir, string outputImageDir,
            string filenameSparse, string filenameDense, bool isLogInitialized);

        private void button3_Click(object sender, EventArgs e)
        {
            render3DResult(imagePath, imagePathOutput, filenameSparse.ToString(), filenameDense.ToString(), isLogInitialized);
        }
    }
}
