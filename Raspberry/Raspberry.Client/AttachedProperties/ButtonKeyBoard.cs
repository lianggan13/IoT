using System.Windows;
using System.Windows.Input;

namespace Raspberry.Client.AttachedProperties
{
    public class ButtonKeyBoard : DependencyObject
    {
        public static Key GetKey(DependencyObject obj)
        {
            return (Key)obj.GetValue(KeyProperty);
        }

        public static void SetKey(DependencyObject obj, int value)
        {
            obj.SetValue(KeyProperty, value);
        }

        // Using a DependencyProperty as the backing store for Key.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty KeyProperty =
            DependencyProperty.RegisterAttached("Key", typeof(Key), typeof(ButtonKeyBoard), new PropertyMetadata(Key.None));

    }
}
