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

namespace SnakeSocksClientGUI
{
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
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

        private void LoadProfileToFrontEnd(Profile p)
        {
            switch_startonrun.IsChecked = p.autorun;
            tb_servaddr.Text = p.servAddr;
            tb_servport.Text = p.servPort;
            tb_prof_name.Text = p.Content.ToString();
            tb_passphrase.Text = p.passphrase;
            tb_module.Text = p.modulePath;
            tb_debug_level.Text = p.debugLevel;
            tb_bindport.Text = p.bindPort;
            tb_bindaddr.Text = p.bindAddr;
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

        }
        private void SaveConfigFile()
        {

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
        {
            Profile prof = listBox.SelectedItem as Profile;
            if(prof == null)
            {
                MessageBox.Show("Error: Selected Item is null.");
                throw new NullReferenceException("null selected item");
            }
            LoadProfileFromFrontEnd(prof);
        }

        private bool validate(Profile prof, string exePath)
        {
            //TODO
            return true;
        }
        private void worker(Profile prof, string exePath)
        {
            Dispatcher.Invoke(delegate
            {
                string exeArg = "-s " + prof.servAddr + " -p " + prof.servPort + " -k " + prof.passphrase + " -L " + prof.bindAddr + " -P " + prof.bindPort + " -D " + prof.debugLevel + " -m " + prof.modulePath;

                if (!validate(prof, exePath))
                    MessageBox.Show("Error: failed to validate your configuration. Check if you've filled all textBoxes and executable do exist.");
                var proc = new Process
                {
                    StartInfo = new ProcessStartInfo
                    {
                        FileName = exePath,
                        Arguments = exeArg,
                        UseShellExecute = false,
                        RedirectStandardOutput = true,
                        CreateNoWindow = true
                    }
                };
                proc.Start();
                while (!proc.StandardOutput.EndOfStream)
                {
                    string line = proc.StandardOutput.ReadLine();
                    display.Text = display.Text + line + "\n";
                }
            });
        }
        private Thread workerThread;
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
            workerThread = new Thread(() => worker(prof, tb_skcli_path.Text));
            workerThread.Start();
        }

        private void on_disable(object sender, RoutedEventArgs e)
        {
            if (workerThread == null)
            {
                MessageBox.Show("Snakesocks is not running.");
                return;
            }
            workerThread.Abort();
            workerThread = null;
        }
    }
}
