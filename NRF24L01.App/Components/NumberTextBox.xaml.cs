using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace NRF24L01.App.Components
{
    /// <summary>
    /// NumberTextBox.xaml 的交互逻辑
    /// </summary>
    partial class NumberTextBox : TextBox
    {
        public int Step
        {
            get { return (int)GetValue(StepProperty); }
            set { SetValue(StepProperty, value); }
        }

        public static readonly DependencyProperty StepProperty =
            DependencyProperty.Register(nameof(Step), typeof(int), typeof(NumberTextBox), new PropertyMetadata(1));

        public int Minimum
        {
            get { return (int)GetValue(MinimumProperty); }
            set { SetValue(MinimumProperty, value); }
        }
        public static readonly DependencyProperty MinimumProperty =
            DependencyProperty.Register(nameof(Minimum), typeof(int), typeof(NumberTextBox), new PropertyMetadata(0));

        public int Maximum
        {
            get { return (int)GetValue(MaximumProperty); }
            set { SetValue(MaximumProperty, value); }
        }
        public static readonly DependencyProperty MaximumProperty =
            DependencyProperty.Register(nameof(Maximum), typeof(int), typeof(NumberTextBox), new PropertyMetadata(5));


        static NumberTextBox()
        {
            DefaultStyleKeyProperty.OverrideMetadata(typeof(NumberTextBox), new FrameworkPropertyMetadata(typeof(NumberTextBox)));
        }

        public NumberTextBox()
        {
            InitializeComponent();
            InputMethod.SetIsInputMethodEnabled(this, false);
        }

        private void NumberTextBox_PreviewTextInput(object sender, TextCompositionEventArgs e)
        {
            e.Handled = new Regex("[^0-9]+").IsMatch(e.Text);
            //if (!e.Handled && int.TryParse(this.Text + e.Text, out int value))
            //{
            //    if (value > Maximum)
            //    {
            //        this.Text = $"{Maximum}";
            //        e.Handled = true;
            //    }
            //    else if (value < Minimum)
            //        this.Text = $"{Minimum}";
            //}
        }

        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate();
            Button PlusButton = (Button)this.Template.FindName("PlusButton", this);
            Button MinusButton = (Button)this.Template.FindName("MinusButton", this);

            PlusButton.Click += PlusButton_MouseLeftButtonDown;
            MinusButton.Click += MinusButton_MouseLeftButtonDown;
        }

        private void PlusButton_MouseLeftButtonDown(object sender, RoutedEventArgs e)
        {
            if (!String.IsNullOrEmpty(this.Text))
            {
                this.Text = int.Parse(this.Text) + Step > Maximum ? $"{Maximum}" : $"{int.Parse(this.Text) + Step}";
                this.SelectionStart = this.Text.Length;
            }
            else
            {
                this.Text = $"{Maximum}";
                this.SelectionStart = this.Text.Length;
            }
        }

        private void MinusButton_MouseLeftButtonDown(object sender, RoutedEventArgs e)
        {
            if (!String.IsNullOrEmpty(this.Text))
            {
                this.Text = int.Parse(this.Text) - Step < Minimum ? $"{Minimum}" : $"{int.Parse(this.Text) - Step}";
                this.SelectionStart = this.Text.Length;
            }
            else
            {
                this.Text = $"{Minimum}";
                this.SelectionStart = this.Text.Length;
            }
        }


    }
}
