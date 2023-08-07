using Prism.Commands;
using Prism.Mvvm;
using Raspberry.Client.AttachedProperties;
using Raspberry.Client.Services;
using Raspberry.Client.Utils;
using System;
using System.Diagnostics;
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

        private string ip = "192.168.1.1";

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

        public DelegateCommand<object> PressKeyCommand => new DelegateCommand<object>(PressKey);


        private ImageSource img;

        public ImageSource Img
        {
            get { return img; }
            set { SetProperty(ref img, value); }
        }

        public MainWindowViewModel()
        {
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
                //System.Drawing.Bitmap bitmap = ImageHelper.Buffer2Bitmap(data);
                //System.Drawing.Image t_img = ImageHelper.AddTextToImg(bitmap, $"{DateTime.Now:HH:mm:ss}", 12.0f, bitmap.Width - 10, bitmap.Height - 10, 120, System.Drawing.Imaging.ImageFormat.Jpeg);

                Application.Current?.Dispatcher.Invoke(() =>
                {
                    Img = ImageHelper.ConvertByteArrayToBitmapImage(data);
                    //Img = ImageHelper.BitmapToBitmapImage(new System.Drawing.Bitmap(t_img));
                });

            }
            catch (Exception ex)
            {
                //Debug.Fail(ex.ToString());
            }
        }

        private void PressKey(object para)
        {
            Button btn = para as Button;
            if (!btn.IsFocused)
            {
                //btn.Focus();
                Keyboard.Focus(btn);
            }


            Key key = ButtonKeyBoard.GetKey(btn);
            Debug.WriteLine($"{nameof(PressKey)}:{key}");
            socketClient.Send($"{key}");
        }


        public void Btn_KeyUp(object sender, KeyEventArgs e)
        {
            if (sender is Button btn)
            {
                Key key = ButtonKeyBoard.GetKey(btn);
                if (key == e.Key)
                {
                    //(Application.Current.MainWindow as MainWindow).imgHost.Focus();
                    Debug.WriteLine($"{nameof(Btn_KeyUp)}:{key}");
                    socketClient.Send($"P");
                }
            }
            e.Handled = true;
        }
    }
}
