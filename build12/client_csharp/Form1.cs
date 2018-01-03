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

        string imagePath;// = "E:\\3d\\2017_03-5\\";
        string imagePathOutput;// = "E:\\3d\\output\\";
        StringBuilder filenameSparse = new StringBuilder();
        StringBuilder filenameDense = new StringBuilder();
        bool isLogInitialized = false;
        
        public Form1()
        {
            InitializeComponent();
            textBox1.Text = "E:\\3d\\2017_03-5\\";
            for (int i = 0; i < checkedListBox1.Items.Count; i++)
                checkedListBox1.SetItemChecked(i, true);
            comboBox1.Text = "2";
            comboBox2.Text = "5";
            textBox2.Text = "abc";
            textBox3.Text = "E:\\3d\\output\\";
        }

        [DllImport(@"dll_reconstruction.dll", EntryPoint = "kernelReBuildSparse", CallingConvention = CallingConvention.Cdecl)]
        public static extern int kernelReBuildSparse(string inputImageDir, StringBuilder resultString, bool useGPU);

        private void button1_Click(object sender, EventArgs e)
        {
            imagePath = textBox1.Text;
            int ret = kernelReBuildSparse(imagePath, filenameSparse, true);
            isLogInitialized = true;
            label1.Text = "return code:" + ret.ToString() + ", file:" + filenameSparse;
        }

        [DllImport(@"dll_reconstruction.dll", EntryPoint = "kernelReBuildDense", CallingConvention = CallingConvention.Cdecl)]
        public static extern int kernelReBuildDense(string inputImageDir, string filenameSparse, StringBuilder resultString, bool isLogInitialized);

        private void button2_Click(object sender, EventArgs e)
        {
            if(filenameSparse.Length == 0)
            {
                MessageBox.Show("请先执行稀疏重建！");
                return;
            }
            int ret = kernelReBuildDense(imagePath, filenameSparse.ToString(), filenameDense, isLogInitialized);
            label1.Text = "return code:" + ret.ToString() + ", file:" + filenameDense;
        }

        [DllImport(@"dll_view3d.dll", EntryPoint = "render3DResult", CallingConvention = CallingConvention.Cdecl)]
        public static extern int render3DResult(string inputImageDir, string outputImageDir,
            string filenameSparse, string filenameDense, bool isLogInitialized,
            string pColorPoint, string pColorSky, int nSizePoint,
            string pOutputFormat, string pOutputName, int nFPS, int nTimeLength,
            int nWindowWidth, int nWindowHeight);

        private void button3_Click(object sender, EventArgs e)
        {
            if (filenameDense.Length == 0)
            {
                MessageBox.Show("请先执行稀疏/稠密重建！");
                return;
            }

            string fileFormat = "";// = "jpg+gif+avi";
            for (int i = 0; i < checkedListBox1.Items.Count; i++)
            {
                if (checkedListBox1.GetItemChecked(i))
                    fileFormat += checkedListBox1.GetItemText( checkedListBox1.Items[i] );
            }

            string fileName = textBox2.Text;// "abc";
            int fps = comboBox1.SelectedIndex + 1;// 2;
            int length = comboBox2.SelectedIndex + 1;// 5; // 动画长度，单位：秒
            int frameTotal = fps * length;
            imagePathOutput = textBox3.Text;
            int ret = render3DResult(imagePath, imagePathOutput, filenameSparse.ToString(), filenameDense.ToString(), isLogInitialized,
            "(0,255,0)", "(0,0,0)", 3,
            fileFormat, fileName, fps, length,
            1280, 1080);

            // 显示图片 gif或jpg，avi/mp4视频需要EmguCV库暂时没引用
            if ( fileFormat.Contains("gif") )
            {
                string imageOutput = imagePathOutput + fileName + ".gif";
                pictureBox1.Image = Image.FromFile(imageOutput);
            }
            else if (fileFormat.Contains("jpg") )
            {
                string imageOutput = imagePathOutput + fileName + frameTotal.ToString() + ".jpg";
                pictureBox1.Image = Image.FromFile(imageOutput);
            }
        }
    }
}
