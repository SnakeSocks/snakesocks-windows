using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Diagnostics;
using System.Threading;
using System.Xml;

namespace SnakeSocksClientGUI
{
    /// <summary>
    /// MainWindow.xaml Logic
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            try
            {
                LoadConfigFile();
            }
            catch(System.IO.FileNotFoundException e)
            {
                // do nothing.
            }
            //Deal auto-enable
            foreach (Profile prof in listBox.Items)
            {
                if (prof.autorun)
                {
                    listBox.SelectedItem = prof;
                    on_enable(null, null);
                }
            }
        }
        ~ MainWindow()
        {
            if (skcli_nogui != null)
                skcli_nogui.Kill();
        }
        private void on_filepick_clicked(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.DefaultExt = ".exe";
            dlg.Filter = "Executable (*.exe)|*.exe|All Files (*.*)|*.*";
            Nullable<bool> result = dlg.ShowDialog();
            if (result == true)
            {
                tb_skcli_path.Text = dlg.FileName;
            }
        }

        private void enable_profile_editor()
        {
            btn_en_proxy.IsEnabled = true;
            btn_dis_proxy.IsEnabled = true;
            btn_rmprof.IsEnabled = true;
            switch_startonrun.IsEnabled = true;
            tb_servaddr.IsEnabled = true;
            tb_servport.IsEnabled = true;
            tb_prof_name.IsEnabled = true;
            tb_passphrase.IsEnabled = true;
            tb_module.IsEnabled = true;
            tb_debug_level.IsEnabled = true;
            tb_bindport.IsEnabled = true;
            tb_bindaddr.IsEnabled = true;
        }

        private void disable_profile_editor()
        {
            btn_en_proxy.IsEnabled = false;
            btn_dis_proxy.IsEnabled = false;
            btn_rmprof.IsEnabled = false;
            switch_startonrun.IsEnabled = false;
            tb_servaddr.IsEnabled = false;
            tb_servport.IsEnabled = false;
            tb_prof_name.IsEnabled = false;
            tb_passphrase.IsEnabled = false;
            tb_module.IsEnabled = false;
            tb_debug_level.IsEnabled = false;
            tb_bindport.IsEnabled = false;
            tb_bindaddr.IsEnabled = false;
        }

        private bool isLoadingFrontend = false;
        private void LoadProfileToFrontEnd(Profile p)
        {
            isLoadingFrontend = true;
            switch_startonrun.IsChecked = p.autorun;
            tb_servaddr.Text = p.servAddr;
            tb_servport.Text = p.servPort;
            tb_prof_name.Text = p.Content.ToString();
            tb_passphrase.Text = p.passphrase;
            tb_module.Text = p.modulePath;
            tb_debug_level.Text = p.debugLevel;
            tb_bindport.Text = p.bindPort;
            tb_bindaddr.Text = p.bindAddr;
            isLoadingFrontend = false;
        }
        private void LoadProfileFromFrontEnd(Profile p)
        {
            p.autorun = switch_startonrun.IsChecked ?? false;
            p.servAddr = tb_servaddr.Text;
            p.servPort = tb_servport.Text;
            p.Content = tb_prof_name.Text;
            p.passphrase = tb_passphrase.Text;
            p.modulePath = tb_module.Text;
            p.debugLevel = tb_debug_level.Text;
            p.bindPort = tb_bindport.Text;
            p.bindAddr = tb_bindaddr.Text;
        }
        private void ClearFrontEnd()
        {
            switch_startonrun.IsChecked = false;
            tb_servaddr.Text = "";
            tb_servport.Text = "";
            tb_prof_name.Text = "";
            tb_passphrase.Text = "";
            tb_module.Text = "";
            tb_debug_level.Text = "";
            tb_bindport.Text = "";
            tb_bindaddr.Text = "";
        }
        private void LoadConfigFile()
        {
            XmlReader x = null;
            try
            {
                XmlReaderSettings settings = new XmlReaderSettings();
                x = XmlReader.Create("skgui.xml", settings);
                x.ReadStartElement("root");
                tb_skcli_path.Text = x.ReadElementString("skcli");
                while (true)
                {
                    try
                    {
                        x.ReadStartElement("profile");
                    }
                    catch (XmlException e)
                    { //Read done.
                        break;
                    }
                    Profile prof = new Profile();
                    prof.Content = x.ReadElementString("name");
                    prof.servAddr = x.ReadElementString("servaddr");
                    prof.servPort = x.ReadElementString("servport");
                    prof.bindAddr = x.ReadElementString("bindaddr");
                    prof.bindPort = x.ReadElementString("bindport");
                    prof.debugLevel = x.ReadElementString("debuglevel");
                    prof.passphrase = x.ReadElementString("passphrase");
                    prof.modulePath = x.ReadElementString("module");
                    prof.autorun = x.ReadElementString("autoenable") == "True";
                    listBox.Items.Add(prof);
                    x.ReadEndElement();
                }
                x.ReadEndElement();
                x.Close();
            }
            finally
            {
                if (x != null)
                    x.Close();
            }

        }
        private void SaveConfigFile()
        {
            XmlWriter x = null;
            try
            {
                XmlWriterSettings settings = new XmlWriterSettings();
                settings.Indent = true;
                x = XmlWriter.Create("skgui.xml", settings);
                x.WriteStartElement("root");
                x.WriteElementString("skcli", tb_skcli_path.Text);
                foreach (Profile prof in listBox.Items)
                {
                    if (String.IsNullOrEmpty(prof.Content.ToString()))
                        continue;
                    x.WriteStartElement("profile");
                    x.WriteElementString("name", prof.Content.ToString());
                    x.WriteElementString("servaddr", prof.servAddr);
                    x.WriteElementString("servport", prof.servPort);
                    x.WriteElementString("bindaddr", prof.bindAddr);
                    x.WriteElementString("bindport", prof.bindPort);
                    x.WriteElementString("debuglevel", prof.debugLevel);
                    x.WriteElementString("passphrase", prof.passphrase);
                    x.WriteElementString("module", prof.modulePath);
                    x.WriteElementString("autoenable", prof.autorun.ToString());
                    x.WriteEndElement();
                }
                x.WriteEndElement();
                x.Flush();
                x.Close();
            }
            finally
            {
                if (x != null)
                    x.Close();
            }
        }
        private class Profile : ListBoxItem
        {
            public string servAddr;
            public string servPort = "8080";
            public string bindAddr = "127.0.0.1";
            public string bindPort = "10802";
            public string passphrase;
            public string debugLevel = "1";
            public string modulePath;
            public bool autorun = false;
        }

        private void on_new_profile(object sender, RoutedEventArgs e)
        {
            Profile prof = new Profile();
            prof.Content = "New Profile";
            listBox.Items.Add(prof);
        }

        private void on_delete_profile(object sender, RoutedEventArgs e)
        {
            ListBoxItem item = (listBox.SelectedItem as ListBoxItem);
            listBox.Items.Remove((item as Profile));
            ClearFrontEnd();
            disable_profile_editor();
        }
        private void on_btn_save_conf(object sender, RoutedEventArgs e)
        {
            SaveConfigFile();
        }

        private void on_listbox_select_changed(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count < 1)
                return;
            enable_profile_editor();
            var item = (e.AddedItems[0] as ListBoxItem);
            Profile prof = item as Profile;
            LoadProfileToFrontEnd(prof);
        }

        private void on_elem_input(object sender, RoutedEventArgs e)
        { //Refresh memory
            if (isLoadingFrontend) //If this event is sent by LoadProfileToFrontend.
                return;
            Profile prof = listBox.SelectedItem as Profile;
            if (prof == null)
            {
                MessageBox.Show("Error: Selected Item is null.");
                throw new NullReferenceException("null selected item");
            }
            LoadProfileFromFrontEnd(prof);
        }
        private void on_autoenable_checked(object sender, RoutedEventArgs e)
        { //Uncheck all other profile
            if (isLoadingFrontend) //If this event is sent by LoadProfileToFrontend.
                return;
            foreach (Profile prof in listBox.Items)
            {
                if (prof.autorun)
                    prof.autorun = false;
            }
            on_elem_input(sender, e);
        }

        private string validate(Profile prof, string exePath)
        {
            int _;

            if (prof.servAddr == "")
                return "server address is required.";
            if (prof.bindAddr == "")
                return "local bind address is required.";
            if (prof.passphrase == "")
                return "passphrase is required.";

            if (prof.modulePath.Contains(' '))
                return "Space is not allowed in modulePath.";
            if (prof.passphrase.Contains(' '))
                return "Space is not allowed in passphrase.";

            if (!System.IO.File.Exists(exePath))
                return "skcli.exe not found.";
            if (!System.IO.File.Exists(prof.modulePath))
                return "module not found.";
            if (!int.TryParse(prof.debugLevel, out _))
                return "Invalid debug level";
            if (!int.TryParse(prof.servPort, out _))
                return "Invalid server port";
            if (!int.TryParse(prof.bindPort, out _))
                return "Invalid local bind port";

            return "";
        }
        private string _exePath; //temp var.
        private void worker(object _prof)
        {
            Profile prof = _prof as Profile;
            string exeArg = "-s " + prof.servAddr + " -p " + prof.servPort + " -k " + prof.passphrase + " -L " + prof.bindAddr + " -P " + prof.bindPort + " -D " + prof.debugLevel + " -m " + prof.modulePath;
            skcli_nogui = new Process
            {
                StartInfo = new ProcessStartInfo
                {
                    FileName = _exePath,
                    Arguments = exeArg,
                    UseShellExecute = false,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    CreateNoWindow = true
                }
            };
            display.Dispatcher.Invoke(() => 
                display.Content += "\n---Launched---\n" + _exePath + " " + exeArg + "\n"
            );

            skcli_nogui.OutputDataReceived += delegate (object sender, DataReceivedEventArgs e)
            {
                try
                {
                    display.Dispatcher.Invoke(() => display.Content += e.Data + "\n", System.Windows.Threading.DispatcherPriority.Background);
                }
                catch (System.Threading.Tasks.TaskCanceledException _) { }
            };
            skcli_nogui.ErrorDataReceived += delegate (object sender, DataReceivedEventArgs e)
            {
                try
                {
                    display.Dispatcher.Invoke(() => display.Content += e.Data + "\n", System.Windows.Threading.DispatcherPriority.Background);
                }
                catch (System.Threading.Tasks.TaskCanceledException _) { }
                //display.Text += e.Data + "\n";
            };
            //TODO: redirect stdout/err
            skcli_nogui.Start();
            skcli_nogui.BeginOutputReadLine();
            skcli_nogui.BeginErrorReadLine();
            //skcli_nogui.WaitForExit();
        }
        private Thread workerThread;
        private Process skcli_nogui;
        private void on_enable(object sender, RoutedEventArgs e)
        {
            Profile prof = listBox.SelectedItem as Profile;
            if (prof == null)
            {
                MessageBox.Show("Error: Selected Item is null.");
                return;
            }
            if (workerThread != null)
            {
                MessageBox.Show("Snakesocks is already running.");
                return;
            }
            string err = validate(prof, tb_skcli_path.Text);
            if (err != "")
            {
                MessageBox.Show("Error: failed to validate your configuration: " + err);
                return;
            }
            _exePath = tb_skcli_path.Text;
            workerThread = new Thread(new ParameterizedThreadStart(worker));
            workerThread.Start(prof);
        }

        private void on_disable(object sender, RoutedEventArgs e)
        {
            if (workerThread == null)
            {
                MessageBox.Show("Snakesocks is not running.");
                return;
            }
            if (skcli_nogui != null)
            {
                skcli_nogui.Kill();
                skcli_nogui = null;
            }
            workerThread = null;
            display.Content += "\n---Stopped---\n";
        }

        private void on_display_copy(object sender, RoutedEventArgs e)
        {
            Clipboard.SetText(display.Content.ToString());
        }

        private void on_display_clear(object sender, RoutedEventArgs e)
        {
            display.Content = "";
        }

        private void show_help(object sender, RoutedEventArgs e)
        {
            MessageBox.Show(@"
SnakeSocks Client GUI for Windows 1.2
Frontend for skcli.exe

This program will start a socks5 server on BindAddress:BindPort(Usually 127.0.0.1:1080). Usually, you must install some extension onto your browser(e.x. SwitchyOmega for Chrome/ium, FoxyProxy for FireFox, and so on) to proxy your packets, and they will manage your connection better.
            ");
        }
    }
}
