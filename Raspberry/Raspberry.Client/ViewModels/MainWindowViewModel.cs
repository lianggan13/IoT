using Prism.Mvvm;
using Prism.Regions;
using Raspberry.Client.AttachedProperties;
using Raspberry.Client.Services;
using Raspberry.Client.Utils;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace Raspberry.Client.ViewModels
{
    public class MainWindowViewModel : BindableBase
    {
        private SocketClient socketClient = new SocketClient();

        private string _title = "Raspberry Application";
        public string Title
        {
            get { return _title; }
            set { SetProperty(ref _title, value); }
        }

        private string ip = "192.168.13.1";

        public string IP
        {
            get { return ip; }
            set { SetProperty(ref ip, value); }
        }

        private bool status = false;

        public bool Status
        {
            get { return status; }
            set { SetProperty(ref status, value); }
        }

        private ImageSource img;

        public ImageSource Img
        {
            get { return img; }
            set { SetProperty(ref img, value); }
        }

        public MainWindowViewModel()
        {
        }

        public MainWindowViewModel(IRegionManager regionManager) : this()
        {
            Keyboard.AddPreviewKeyDownHandler(Application.Current.MainWindow, OnPreviewKeyDown);
            Keyboard.AddPreviewKeyUpHandler(Application.Current.MainWindow, OnPreviewKeyUp);

            this.PropertyChanged += MainViewModel_PropertyChanged;
        }

        private void MainViewModel_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            switch (e.PropertyName)
            {
                case nameof(Status):
                    StatusChanged();
                    break;
            }
        }

        private void StatusChanged()
        {
            try
            {
                if (status)
                {
                    socketClient.Connect(IP, 32769);
                    socketClient.Received += SocketClient_Received;
                }
                else
                {
                    socketClient.Received -= SocketClient_Received;
                    socketClient.Disconnect();
                }
            }
            catch (Exception ex)
            {
                Status = false;
                MessageBox.Show(ex.Message);
            }
        }

        private void SocketClient_Received(object arg1, byte[] data)
        {
            try
            {
                Application.Current?.Dispatcher.BeginInvoke(() =>
                {
                    Img = ImageHelper.Bytes2BitmapImage(data);
                });
            }
            catch (Exception ex)
            {
                Debug.Fail(ex.ToString());
            }
        }

        Lazy<List<Button>> buttons = new Lazy<List<Button>>(() =>
        {
            return UIControlHelper.GetLogicChildObjects<Button>(Application.Current.MainWindow);
        });

        private void OnPreviewKeyDown(object sender, KeyEventArgs e)
        {
            Button btn = e.Source as Button;
            if (btn == null || ButtonKeyBoard.GetKey(btn) != e.Key)
            {
                btn = buttons.Value.SingleOrDefault(b => ButtonKeyBoard.GetKey(b) == e.Key);
                if (btn == null)
                    return;
            }

            if (btn.IsEnabled)
                btn.IsEnabled = false;
            else if (e.Key == Key.W || e.Key == Key.A || e.Key == Key.S || e.Key == Key.D)
                return;

            Debug.WriteLine($"{nameof(OnPreviewKeyDown)}:{e.Key}");
            socketClient.Send($"{e.Key}");
        }

        private void OnPreviewKeyUp(object sender, KeyEventArgs e)
        {
            Button btn = e.Source as Button;
            if (btn == null || ButtonKeyBoard.GetKey(btn) != e.Key)
            {
                btn = buttons.Value.SingleOrDefault(b => ButtonKeyBoard.GetKey(b) == e.Key);
                if (btn == null)
                    return;
            }

            if (!btn.IsEnabled)
                btn.IsEnabled = true;
            else if (e.Key == Key.W || e.Key == Key.A || e.Key == Key.S || e.Key == Key.D)
                return;

            Debug.WriteLine($"{nameof(OnPreviewKeyUp)}:{e.Key}");
            socketClient.Send($"P");
        }
    }
}
