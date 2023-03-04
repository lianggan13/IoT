namespace Raspberry.App.AttachedProperties
{
    public class ButtonKeyBoard : BindableObject
    {
        public static readonly BindableProperty KeyProperty =
    BindableProperty.CreateAttached("Key", typeof(Key), typeof(ButtonKeyBoard), defaultValue: Key.None);

        public static Key GetKey(BindableObject view)
        {
            return (Key)view.GetValue(KeyProperty);
        }

        public static void SetKey(BindableObject view, Key value)
        {
            view.SetValue(KeyProperty, value);
        }
    }

    public enum Key
    {
        None,
        W,
        A,
        S,
        D,
        Up,
        Down,
        Left,
        Right,

    }
}
