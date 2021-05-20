using System;
using System.Collections.Generic;
using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace ConsoleApp1
{
    class Program
    {
        static void Read(String path, out String[] ws)
        {
            ws = null;
            try
            {
                String s;
                using (StreamReader sr = new StreamReader(path))
                {
                    s = sr.ReadToEnd();
                }                
                s=s.Replace("\r", "").Replace(",", "").Replace("\"", "").ToLower();
                ws = s.Split('\n');
                /*
                foreach (var w in ws) {
                    Console.WriteLine(w);
                }
                Console.WriteLine();
                */
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
            }
        }
        static int match(int c1, int c2) {
            if (c1 == c2)
                return 0;
            else
                return 1;
        }
        static int indel(int c) {
            return 1;
        }

        static int string_compare(int[] s1, int[] s2) {
            int MATCH = 0;
            int INSERT = 1;
            int DELETE = 2;
            int[] opt = {0, 0, 0};
            int[,] cost=new int[s1.Length + 1,s2.Length + 1];

            for (int i = 0; i < s1.Length + 1; i++) {
                for (int j = 0; j < s2.Length + 1; j++)
                {
                    if (i == 0)
                        cost[i, j] = j;
                    else if (j == 0)
                        cost[i, j] = i;
                    else
                        cost[i, j] = 0;
                }
            }

            for (int i = 1; i < s1.Length + 1; i++)
            {
                for (int j = 1; j < s2.Length + 1; j++)
                {
                    opt[MATCH] = cost[i - 1, j - 1] + match(s1[i - 1], s2[j - 1]);
                    opt[INSERT] = cost[i, j - 1] + indel(s2[j - 1]);
                    opt[DELETE] = cost[i - 1, j] + indel(s1[i - 1]);
                    cost[i, j] = opt[MATCH];
                    for (int k = 1; k < 3; k++)
                    {
                        if (opt[k] < cost[i, j])
                            cost[i, j] = opt[k];
                    }                    
                }
            }
            return cost[s1.Length, s2.Length];
        }

        static int Chain(String w1, String w2, int len) {            
            var d = new Dictionary<string, int>();
            int count = w1.Length < w2.Length ? w1.Length : w2.Length;
            int[] a1 = new int[w1.Length - len + 1];
            int[] a2 = new int[w2.Length - len + 1];
            for (int i = 0; i < w1.Length - len + 1; i++)
            {
                string key = w1.Substring(i, len);
                int p = d.Count;
                if (d.ContainsKey(key)) p = d[key]; else d[key] = p;
                a1[i] = p;
            }
            for (int i = 0; i < w2.Length - len + 1; i++)
            {
                string key = w2.Substring(i, len);
                int p = d.Count;
                if (d.ContainsKey(key)) p = d[key]; else d[key] = p;
                a2[i] = p;
            }
            return string_compare(a1,a2);
        }
        static double Estimate(String w1, String w2, bool write) {
            int sum = 0;
            int count = w1.Length < w2.Length ? w1.Length : w2.Length;
            for (int i = 1; i <= count; i++)
            {
                int ans = Chain(w1, w2, i);
                if (write) Console.WriteLine(i.ToString() + " " + ans.ToString());
                sum += ans;
            }
            double avg = (double)sum / (count);
            return avg;
        }
        static void Swap(ref String w1, int i1, int j1, int k1)
        {
            var ws=w1.Split(' ');
            String tmp = ws[i1]; ws[i1] = ws[j1]; ws[j1] = tmp;
            tmp = "";
            for (int i = 0; i < ws.Length; i++)
            {
                int k = (i + ws.Length + k1) % ws.Length;
                if (i == 0)
                    tmp = ws[k];
                else
                    tmp += " " + ws[k];
            }
            w1 = tmp;
        }

        static void Shift(ref String w1, int i1)
        {
            var ws = w1.Split(' ');
            String tmp = "";
            for (int i = 0; i < ws.Length; i++)
            {
                int k = (i + ws.Length + i1) % ws.Length;
                if (i == 0)
                    tmp = ws[k];
                else
                    tmp += " " + ws[k];
            }
            w1 = tmp;
        }

        struct Rec {             
            public int i;
            public int j;
            public String w1;
            public String w2;
        }

        static void Display(object obj) {
            Rec r = ((Rec)obj);
            int i = r.i;
            int j = r.j;
            String w1 = r.w1;
            String w2 = r.w2;
            
            double best = Estimate(w1, w2, false);

            dist[i, j] = best;
            dist[j, i] = dist[i, j];

            return;
            /*
            bool improvement;
            int w1Length = w1.Split(' ').Length;
            int w2Length = w2.Split(' ').Length;
            String tmp1, tmp2;
            do
            {
                improvement = false;
                tmp1 = w1;
                tmp2 = w2;
                for (int i1 = 0; i1 < w1Length; i1++)
                {
                    for (int j1 = i1; j1 < w1Length; j1++)
                    {
                        for (int k1 = -1; k1 < 2; k1++)
                        {
                            Swap(ref w1, i1, j1, k1);
                            for (int i2 = 0; i2 < w2Length; i2++)
                            {
                                for (int j2 = i2; j2 < w2Length; j2++)
                                {
                                    for (int k2 = -1; k2 < 2; k2++)
                                    {
                                        Swap(ref w2, i2, j2, k2);
                                        Console.WriteLine(w1);
                                        Console.WriteLine(w2);
                                        double avg = Estimate(w1, w2, true);
                                        if (best > avg)
                                        {
                                            best = avg;
                                            improvement = true;
                                        }
                                        if (improvement) break;
                                        w2 = tmp2;
                                    }
                                    if (improvement) break;
                                }
                                if (improvement) break;
                            }
                            if (improvement) break;
                            w1 = tmp1;
                        }
                        if (improvement) break;
                    }
                    if (improvement) break;
                }
            } while (improvement);
            tmp1 = w1;
            tmp2 = w2;
            for (int i1 = 0; i1 < w1Length; i1++)
            {
                for (int i2 = 0; i2 < w2Length; i2++)
                {
                    Shift(ref w1, i1);
                    Shift(ref w2, i2);
                    double avg = Estimate(w1, w2, false);
                    if (best > avg)
                    {
                        best = avg;
                        improvement = true;
                        Console.WriteLine(w1);
                        Console.WriteLine(w2);
                        Console.WriteLine("Estimate = " + best.ToString());
                    }
                    w1 = tmp1;
                    w2 = tmp2;
                }
            }

            dist[i, j] = best;
            dist[j, i] = dist[i, j];
            */
        }

        static void FindFreeCore(Rec r)
        {
            do
            {
                for (int i = 0; i < Threads.Length; i++)
                {
                    if (Threads[i]==null || !Threads[i].IsAlive)
                    {
                        Threads[i] = new Thread(new ParameterizedThreadStart(Display));
                        Threads[i].Start(r);
                        return;
                    }
                }
                Thread.Sleep(1);
            } while (true);
        }
        static void Main(string[] args)
        {            
            
            int pc = Environment.ProcessorCount;
            pc = 2;
            Console.WriteLine(pc.ToString());
            String[] ws;
            Read("input.txt", out ws);
            int count = ws.Length;
            dist = new double[count, count];
            Threads = new Thread[pc-1];           
            for (int i = 0; i < count; i++)
            {
                for (int j = i + 1; j < count; j++)
                {                    
                    Console.WriteLine(i.ToString()+" "+j.ToString());
                    String w1 = ws[i];
                    String w2 = ws[j];
                    Rec r;
                    r.i = i;
                    r.j = j;
                    r.w1 = w1;
                    r.w2 = w2;
                    FindFreeCore(r);
                }
            }

            bool alive = true;
            while (alive)
            {
                alive = false;
                for (int i = 0; i < Threads.Length; i++)
                {
                    if (Threads[i] != null && Threads[i].IsAlive)
                    {
                        alive = true;
                        break;
                    }
                }
                Thread.Sleep(1);
            }
            double[] sum = new double[count];
            using (StreamWriter outputFile = new StreamWriter("matrix.txt"))
            {                
                for (int i = 0; i < count; i++)
                {
                    String s = "";
                    sum[i] = 0;
                    for (int j = 0; j < count; j++)
                    {
                        s += dist[i, j].ToString("0.000") + " ";
                        sum[i] += dist[i, j];
                    }
                    //Console.WriteLine(s);
                    outputFile.WriteLine(s);
                }
            }
            SortedDictionary<double, String> dict = new SortedDictionary<double, string>();

            using (StreamWriter outputFile = new StreamWriter("dist.txt")) { 
                for (int i = 0; i < count; i++)
                {
                    Console.WriteLine(sum[i].ToString("0.00000000000000") + "   " + ws[i]);
                    outputFile.WriteLine(sum[i].ToString("0.00000000000000") + "   " + ws[i]);
                    dict[sum[i]] = ws[i];
                }
            }

            using (StreamWriter outputFile = new StreamWriter("dist2.txt"))
            {
                foreach (var d in dict)
                {
                    Console.WriteLine(d.Key.ToString("0.00000000000000") + "   " + d.Value);
                    outputFile.WriteLine(d.Key.ToString("0.00000000000000") + "   " + d.Value);                                        
                }
            }            
        }
        static Thread[] Threads;
        static double[,] dist;
    }
}
