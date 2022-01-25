using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;

namespace WindowsFormsApp1
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
            chart1.Visible = false;
        }

        public List<String> headlines;
        public HashSet<String> brands;
        public HashSet<String> groups;
        public List<String[]> headlines2;
        //public List<String[]> headlines3;
        public SortedDictionary<String, int> dict;
        public List<double[]> metric;

        private void button1_Click(object sender, EventArgs e)
        {
            chart1.Visible = false;
            string line;
            string s="";
            headlines = new List<string>();
            System.IO.StreamReader file = new System.IO.StreamReader(textBox4.Text);
            while ((line = file.ReadLine()) != null)
            {
                if (line.Length == 0) continue;
                if (line[0]=='\"' || line[0] == '\'')
                    line = line.Substring(1, line.Length-2);
                line = line.ToLower();
                s = s + line + "\n";
                headlines.Add(line);
            }
            file.Close();
            brands = new HashSet<string>();
            richTextBox1.Text = s;
            System.IO.StreamReader file2 = new System.IO.StreamReader("Brands.txt");
            while ((line = file2.ReadLine()) != null)
            {
                if (line.Length == 0) continue;
                line = line.Substring(line.IndexOf(',') + 2, line.Length - line.IndexOf(',') - 2);
                if (line[0] == '\"' || line[0] == '\'')
                    line = line.Substring(1, line.Length - 2);
                line = line.ToLower();
                var b = line.Split(' ');
                foreach(var t in b)
                    brands.Add(t);
            }
            file2.Close();
            groups = new HashSet<string>();
            System.IO.StreamReader file3 = new System.IO.StreamReader("Groups.txt");
            while ((line = file3.ReadLine()) != null)
            {
                if (line.Length == 0) continue;
                line = line.Substring(line.IndexOf(',') + 2, line.Length - line.IndexOf(',') - 2);
                if (line[0] == '\"' || line[0] == '\'')
                    line = line.Substring(1, line.Length - 2);
                if (line.Length == 0) continue;
                if (line[0] == '{')
                    line = line.Substring(1, line.Length - 2);
                line = line.ToLower();
                var b = line.Split(' ');
                foreach (var t in b)
                    groups.Add(t);
            }
            file3.Close();
        }

        private void button2_Click(object sender, EventArgs e)
        {
            chart1.Visible = false;
            dict = new SortedDictionary<string, int>();
            headlines2 = new List<string[]>();
            String s = "";
            foreach (var line in headlines)
            {
                var list = line.Split(' ');
                foreach(var w in list)
                {
                    bool b = !brands.Contains(w);
                    bool g = !groups.Contains(w);
                    if (w.Length>0 && b && g)
                    {
                        if (dict.ContainsKey(w))
                            dict[w]++;
                        else
                            dict.Add(w,1);
                        s += w + " ";
                    }
                }
                s += "\n";
                headlines2.Add(list);
            }
            richTextBox1.Text = s;

            headlines.Clear();
            foreach (var list in headlines2)
            {
                String s1 = "";
                foreach (var w in list)
                {
                    bool b = !brands.Contains(w);
                    bool g = !groups.Contains(w);
                    if (w.Length > 0 && b && g)
                    {
                        s1 += w + " ";
                    }
                }
                headlines.Add(s1);
            }
            s = "";
            foreach (var rec in dict)
            {
                double r = Math.Log(headlines2.Count / rec.Value) + Math.Log((headlines2.Count - rec.Value) / rec.Value);
                if (r < 0) r = 0;
                //double r = Math.Log(headlines2.Count() / rec.Value);
                s = s + rec.Key + ": " + rec.Value.ToString() + ": " + r + "\n";
            }
            richTextBox2.Text = s;
        }

        private void button3_Click(object sender, EventArgs e)
        {
            chart1.Visible = false;
            metric = new List<double[]>();
            var d = dict.ToArray();
            int n = dict.Count;
            double alpha = double.Parse(textBox2.Text);
            foreach (var list in headlines2)
            {
                double[] r = new double[n];
                SortedSet<string> tmp = new SortedSet<string>();
                foreach (var w in list)
                {                    
                    tmp.Add(w);
                }
                for (var i = 0; i < n; i++)
                {
                    if (tmp.Contains(d[i].Key))
                    {
                        //r[i] = Math.Log((headlines2.Count - d[i].Value + 0.5) / (d[i].Value + 0.5));
                        double t1 = alpha * Math.Log(headlines2.Count / d[i].Value) + (1 - alpha) * Math.Log((headlines2.Count - d[i].Value) / d[i].Value);
                        //double t1 = Math.Abs(headlines2.Count/2 - d[i].Value) / d[i].Value;
                        //if (t1 <= 0) t1 = 1;
                        //t1 = Math.Log(t1);
                        //double t1 = Math.Log((headlines2.Count - d[i].Value) / d[i].Value);

                        if (t1 < 0) t1 = 0;
                        r[i] = t1;
                    }
                    else
                    {
                        r[i] = 0;
                    }
                }
                metric.Add(r);
            }           
            
            for (var i = 0; i < headlines2.Count; i++)
            {
                double r = 0;
                for (var j = 0; j < n; j++)
                {
                    r += metric[i][j];
                }
                if (r > 0)
                {
                    for (var j = 0; j < n; j++)
                    {
                        metric[i][j] /= r;
                    }
                }                
            }          
            if (n < 50)
            {
                String s = "";
                for (var i = 0; i < headlines2.Count; i++)
                {
                    s += i.ToString() + ": ";
                    for (var j = 0; j < n; j++)
                    {
                        s += metric[i][j].ToString() + ", ";
                    }
                    s += "\n";
                }
                richTextBox2.Text = s;
            }           
        }
        public double[,] val;
        public int count;

        private void BFS(List<int>[] graph, int start, out int last, out int d)
        {
            //int deep = int.Parse(textBox7.Text);
            SortedSet<int> flag = new SortedSet<int>();
            Queue<int> q = new Queue<int>();
            Queue<int> depth = new Queue<int>();
            flag.Add(start);
            q.Enqueue(start);
            depth.Enqueue(1);
            d = 0;
            last = start;
            while (q.Count > 0)
            {
                int p = q.Dequeue();
                int d1 = depth.Dequeue();
                if (d < d1) {
                    d = d1;
                    last = p;
                }
                for (int i = 0; i < graph[p].Count; i++)
                {
                    int g = graph[p][i];
                    if (!flag.Contains(g))
                    {
                        flag.Add(g);
                        q.Enqueue(g);
                        depth.Enqueue(d1 + 1);
                    }
                }                
            }                
        }
        private double ComponentCount(List<int>[] graph)
        {
            //int deep = int.Parse(textBox7.Text);
            bool[] flag = new bool[count];
            for (int i = 0; i < count; i++)
            {
                flag[i] = false;
            }
            Queue<int> q = new Queue<int>();           
            int k = 0;
            String s = "";
            List<int> ans = new List<int>();
            int max = 0;
            double count2 = 0;
            //int One = 0;
            while (true)
            {
                int start = -1;
                for (int i = 0; i < count; i++)
                {
                    if (!flag[i])
                    {
                        start = i;
                        break;
                    }
                }
                if (start == -1) break;
                k++;
                List<int> a = new List<int>();
                flag[start] = true;
                q.Enqueue(start);               
                int c = 0;
                while (q.Count > 0)
                {
                    c++;
                    int p = q.Dequeue();
                    a.Add(p);
                    s += headlines[p] + "\n";
                    for (int i = 0; i < graph[p].Count; i++)
                    {
                        int g = graph[p][i];
                        if (!flag[g])
                        {
                            flag[g] = true;
                            q.Enqueue(g);
                        }
                    }
                }
                int tmp,last, d;
                BFS(graph, start, out tmp, out d);
                BFS(graph, tmp, out last, out d);
                if (max < c)
                {
                    max = c;
                    ans = a;
                }
                //if (c > 1)
                if (c > 0)
                {
                    double dd = d;
                    dd /= c;
                    count2 += dd;
                }
                else {
                    //One = 1;
                }
                s += "\n\n\n\n\n\n";
                /*
                if (c == 1)
                {
                    s += "\n" + headlines[a[0]] + "\n";
                }
                else
                {
                    s += " " + c.ToString() + "\n";
                }*/
            }            
            s += "\n" + k.ToString() + "\n";
            /*
            foreach (var v in ans)
            {
                s += headlines[v] + "\n";
            }
            */
            //count2 += One;
            s += "\n" + count2.ToString() + "\n";
            richTextBox1.Text = s;
            //return k;
            return count2;
        }
        private double barrier()
        {
            //double barrier1 = double.Parse(textBox2.Text);
            double barrier2 = double.Parse(textBox3.Text);
            List<int>[] graph = new List<int>[count];
            for (int i = 0; i < count; i++)
            {
                graph[i] = new List<int>();
                for (int j = 0; j < count; j++)
                    //if (i != j && /*barrier1 <= (1 - val[i, j]) &&*/ (1 - val[i, j]) <= barrier2)
                    if (i != j && val[i, j] >= barrier2)
                    {
                        graph[i].Add(j);
                    }
            }
            return ComponentCount(graph);
        }
        
        private void button5_Click(object sender, EventArgs e)
        {
            chart1.Visible = false;
            int n = dict.Count;
            count = metric.Count;
            val = new double[count, count];
            for (int i = 0; i < count; i++)
            {
                for (int j = 0; j < count; j++)
                {
                    double s = 0;
                    for (int k = 0; k < n; k++) {
                        s += metric[i][k] * metric[j][k];
                        //double t = metric[i][k] - metric[j][k];
                        //s += t*t;
                    }
                    val[i, j] = s; //Math.Sqrt(s) / n;
                } 
            }
            barrier();
        }
        private double ff(double x)
        {
            textBox3.Text = x.ToString();
            double count = double.Parse(textBox5.Text);
            double result = barrier();
            double delta = result - count;
            return delta;
        }
        private double GoldenSearch(double lx, double rx, double EpsA)
        {
            double Gold = (Math.Sqrt(5.0) - 1.0) / 2;
            double HalfEps = EpsA * 0.5;
            double a, b, t, step;
            a = lx; b = rx;
            step = (b - a) * (1 - Gold);
            t = a + step;
            double direction, f, fn;
            direction = 1.0;
            f = ff(t);
            int itc = 0;
            int MaxI = 100000;
            double prevdir = direction;
            while ((step > HalfEps) && (itc < MaxI))
            {
                step = Gold * step;
                t += step * direction;
                fn = ff(t);
                direction = fn < 0 ? 1 : -1;
                if (prevdir != direction)
                {                    
                    t += step * direction;
                } else {
                    f = fn;
                }
                prevdir = direction;
                itc++;
            }
            return f;
        }
        
        private void button4_Click(object sender, EventArgs e)
        {
            chart1.Visible = false;
            GoldenSearch(0, 1, 1.0E-16);
        }

        private void button6_Click(object sender, EventArgs e)
        {
            chart1.Visible = true;
            chart1.Series[0].Points.Clear();
            chart1.Series[1].Points.Clear();
            int max=int.Parse(textBox6.Text);
            int step = max / 100;
            for (int i = 1; i <= max; i += step)
            {
                textBox5.Text = i.ToString();
                double x = i;
                double y = GoldenSearch(0, 1, 1.0E-16) + i;
                chart1.Series[0].Points.AddXY(x, y);
                chart1.Series[1].Points.AddXY(x, x);
            }
        }

        private void hScrollBar1_Scroll(object sender, ScrollEventArgs e)
        {
            textBox2.Text = ((double)(hScrollBar1.Value - hScrollBar1.Minimum) / (hScrollBar1.Maximum - hScrollBar1.Minimum)).ToString();
        }

        private void textBox2_TextChanged(object sender, EventArgs e)
        {
            try
            {
                double alpha = double.Parse(textBox2.Text);
                hScrollBar1.Value = (int)(hScrollBar1.Minimum + alpha * (hScrollBar1.Maximum - hScrollBar1.Minimum));
            }
            catch (Exception)
            {
            }
        }
    }
}
