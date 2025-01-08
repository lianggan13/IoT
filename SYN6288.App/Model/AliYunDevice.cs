using System.ComponentModel;

namespace SYN6288.App.Model
{
    public class AliYunDevice : INotifyPropertyChanged
    {
        #region NotifyPropertyChanged
        public event PropertyChangedEventHandler PropertyChanged;

        /// <summary>
        /// 属性改变时触发
        /// </summary>
        /// <param name="propertyName"></param>
        public virtual void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }


        /// <summary>
        /// 设置需通知的属性的值
        /// </summary>
        /// <typeparam name="T">属性的类型</typeparam>
        /// <param name="item">属性</param>
        /// <param name="value">值</param>
        /// <param name="propertyName">属性名称</param>
        protected virtual void SetProperty<T>(ref T item, T value, [System.Runtime.CompilerServices.CallerMemberName] string propertyName = null)
        {
            if (!EqualityComparer<T>.Default.Equals(item, value))
            {
                item = value;
                OnPropertyChanged(propertyName);
            }
        }

        #endregion

        private int led = 1;
        public int Led
        {
            get { return led; }
            set { SetProperty(ref led, value); }
        }

        private double humi;
        public double Humi
        {
            get { return humi; }
            set { SetProperty(ref humi, value); }
        }

        private double temp;
        public double Temp
        {
            get { return temp; }
            set { SetProperty(ref temp, value); }
        }
    }
}
