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

        [DllImport(@"dll_reconstruction.dll", EntryPoint = "kernelReBuildReady", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool kernelReBuildReady( );
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

            button4.BackColor = Color.Black;    // 背景色
            button5.BackColor = Color.Lime;     // 点云色

            comboBox3.Text = "3";               // 点大小
            comboBox4.SelectedIndex = 1;        // 是否统一色

            textBox4.Text = "1280";             // 分辨率宽，单位：像素
            textBox5.Text = "1080";             // 分辨率高，单位：像素
            textBox6.Text = "2";                // 线程数目
            textBox7.Text = "35";               // 噪声滤除参数，值越高滤除越多 

            comboBox5.SelectedIndex = 1;        // 特征密度，0小，1略小，2中，3略大，4大

            checkBox1.Checked = true;           // 开启GPU

            isLogInitialized = kernelReBuildReady();
        }

        [DllImport(@"dll_reconstruction.dll", EntryPoint = "kernelReBuildSparse", CallingConvention = CallingConvention.Cdecl)]
        public static extern int kernelReBuildSparse(string inputImageDir, StringBuilder resultString, bool useGPU,
            int num_threads, int feature_density, bool match_out_of_core);

        private void button1_Click(object sender, EventArgs e)
        {
            if ( !isLogInitialized )
                isLogInitialized = kernelReBuildReady();

            imagePath = textBox1.Text;
            bool bUseGPU = checkBox1.Checked;
            int nThreads;
            int.TryParse(textBox6.Text, out nThreads);
            if (nThreads > 2)
                nThreads = 2;// 线程数>=3，内存接近1g，有时程序会崩溃
            int nFeatureDensity = comboBox5.SelectedIndex;
            bool bMatchOutOfCore = checkBox2.Checked;
            DateTime dt = DateTime.Now;

            int ret = kernelReBuildSparse(imagePath, filenameSparse, bUseGPU, nThreads, nFeatureDensity, bMatchOutOfCore);

            string timeString = DateTime.Now.Subtract(dt).TotalSeconds.ToString();

            string retString = "return code:" + ret.ToString() + ", file:" + filenameSparse + ", time(s):" + timeString;
            MessageBox.Show(retString);

        }

        [DllImport(@"dll_reconstruction.dll", EntryPoint = "kernelReBuildDense", CallingConvention = CallingConvention.Cdecl)]
        public static extern int kernelReBuildDense(string inputImageDir, string filenameSparse, StringBuilder resultString, bool isLogInitialized,
            bool bUndistort, int noise_removal );

        private void button2_Click(object sender, EventArgs e)
        {
            if(filenameSparse.Length == 0)
            {
                MessageBox.Show("请先执行稀疏重建！");
                return;
            }

            int nNoise;
            int.TryParse(textBox7.Text, out nNoise);
            bool bUndistort = checkBox3.Checked;
            DateTime dt = DateTime.Now;

            int ret = kernelReBuildDense(imagePath, filenameSparse.ToString(), filenameDense, isLogInitialized,
                bUndistort, nNoise);

            string timeString = DateTime.Now.Subtract(dt).TotalSeconds.ToString();
            string retString = "return code:" + ret.ToString() + ", file:" + filenameDense + ", time(s):" + timeString;
            MessageBox.Show(retString);
        }

        private string color2string(Color rgb)
        {
            string strColorSky = "(";
            strColorSky += rgb.R + ",";
            strColorSky += rgb.G + ",";
            strColorSky += rgb.B + ")";
            return strColorSky;
        }

        [DllImport(@"dll_view3d.dll", EntryPoint = "render3DResult", CallingConvention = CallingConvention.Cdecl)]
        public static extern int render3DResult(string inputImageDir, string outputImageDir,
            string filenameSparse, string filenameDense, bool isLogInitialized,
            string pColorSky, string pColorPoint, bool bSameColor, int nSizePoint,
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
            int nPointSize = comboBox3.SelectedIndex + 1;// 3;  // 点大小
            bool bSameColor = comboBox4.SelectedIndex == 1;          // 是否统一色
            string strColorSky = color2string(button4.BackColor);
            string strColorPoint = color2string(button5.BackColor);

            int nWidth, nHeight;
            int.TryParse(textBox4.Text, out nWidth);
            int.TryParse(textBox5.Text, out nHeight);
            DateTime dt = DateTime.Now;
 
            int ret = render3DResult(imagePath, imagePathOutput, filenameSparse.ToString(), filenameDense.ToString(), isLogInitialized,
            strColorSky, strColorPoint, bSameColor, nPointSize,
            fileFormat, fileName, fps, length,
            nWidth, nHeight);

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

            string timeString = DateTime.Now.Subtract(dt).TotalSeconds.ToString();
            string retString = "return code:" + ret.ToString() + ", file:" + imagePathOutput + fileName + ", time(s):" + timeString;
            MessageBox.Show(retString);
        }

        private void button4_Click(object sender, EventArgs e)
        {
            ColorDialog ColorForm = new ColorDialog();
            if (ColorForm.ShowDialog() == DialogResult.OK)
            {
                Color GetColor = ColorForm.Color;
                //GetColor就是用户选择的颜色，接下来就可以使用该颜色了
                button4.BackColor = GetColor;
            }
        }

        private void button5_Click(object sender, EventArgs e)
        {
            ColorDialog ColorForm = new ColorDialog();
            if (ColorForm.ShowDialog() == DialogResult.OK)
            {
                Color GetColor = ColorForm.Color;
                //GetColor就是用户选择的颜色，接下来就可以使用该颜色了
                button5.BackColor = GetColor;
            }
        }
    }
}
