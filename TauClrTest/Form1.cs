using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using TauClr;

namespace TauClrTest
{
    public partial class Form1 : Form
    {
        TauSharp tau;
        SoundFontSharp sf;
        public Form1()
        {
            InitializeComponent();

            string str = System.Windows.Forms.Application.StartupPath;
            int end = str.LastIndexOf('\\');
            end = str.LastIndexOf('\\', end - 1);
            string rootPath = str.Substring(0, end);

            //midi文件路径
            string midiPath = rootPath + "\\TauTest\\midi\\";

            //soundfont音源路径
            string sfPath = rootPath + "\\TauTest\\SoundFont\\";

            sf = new SoundFontSharp();
            sf.Parse("SF2", sfPath + "GeneralUser GS MuseScore v1.442.sf2");


            tau = new TauSharp();
            tau.SetSoundFont(sf);
            tau.Open();

            string p = midiPath + "aloha heja he-Achim Reichel.mid";

            //非阻塞加载方式
            //tau.Load(p, false);
            //while (!tau.IsLoadCompleted())
            //    Thread.Sleep(5);
            //tau.Play();

            //阻塞加载方式
            tau.Load(p);
            tau.Play();


            //
            // art();

        }

        public void art()
        {
            InstrumentSharp inst = tau.EnableInstrument(0, 0, 0, 0);
            tau.OnKey(60, 127, inst);
            tau.OffKey(60, 127, inst, 2000);
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            tau.Dispose();
            sf.Dispose();
        }
    }
}
