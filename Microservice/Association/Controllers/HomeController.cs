using Association.Models;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Logging;
using MySql.Data.MySqlClient;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading.Tasks;
using Tutorial.MySiteUtils;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;

namespace Association.Controllers
{
    public class HomeController : Controller
    {
        private readonly ILogger<HomeController> _logger;

        public HomeController(ILogger<HomeController> logger)
        {
            _logger = logger;
        }

        static int match(char c1, char c2)
        {
            if (c1.ToString().ToUpper() == c2.ToString().ToUpper())
                return 0;
            else
                return 1;
        }
        static int indel(char c)
        {
            return 1;
        }
        static double string_compare(string s1, string s2, out string mask)
        {
            const int MATCH = 0;
            const int INSERT = 1;
            const int DELETE = 2;
            int[] opt = { 0, 0, 0 };
            int[,] cost = new int[s1.Length + 1, s2.Length + 1];
            int[,] move = new int[s1.Length + 1, s2.Length + 1];
            int i, j;
            for (i = 0; i < s1.Length + 1; i++)
            {
                for (j = 0; j < s2.Length + 1; j++)
                {
                    if (i == 0)
                    {
                        cost[i, j] = j;
                        move[i, j] = INSERT;
                    }
                    else if (j == 0)
                    {
                        cost[i, j] = i;
                        move[i, j] = DELETE;
                    }
                    else
                    {
                        cost[i, j] = 0;
                        move[i, j] = MATCH;
                    }
                }
            }

            for (i = 1; i < s1.Length + 1; i++)
            {
                for (j = 1; j < s2.Length + 1; j++)
                {
                    opt[MATCH] = cost[i - 1, j - 1] + match(s1[i - 1], s2[j - 1]);
                    opt[INSERT] = cost[i, j - 1] + indel(s2[j - 1]);
                    opt[DELETE] = cost[i - 1, j] + indel(s1[i - 1]);
                    cost[i, j] = opt[MATCH];
                    for (int k = 1; k < 3; k++)
                    {
                        if (opt[k] < cost[i, j])
                        {
                            cost[i, j] = opt[k];
                            move[i, j] = k;
                        }
                    }
                }
            }

            i = s1.Length;
            j = s2.Length;
            mask = "";
            while (i>0 || j>0)
            {
                switch (move[i, j]) {
                    case MATCH: if (cost[i, j] == cost[i - 1, j - 1]) mask = "C" + mask; else mask = "I" + mask; i--; j--; break;
                    case INSERT: mask = "+" + mask; j--; break;
                    case DELETE: mask = "-" + mask; i--; break;
                }
            }
            return Math.Exp(-cost[s1.Length, s2.Length]);
        }

        public JsonResult Index(int id=-1, int deep = 1)
        {
            string mask;
            if (id == -1)
            {
                return Json(0);
            }
            SiteUtils su = new SiteUtils();
            string res;
            su.ReadSiteIdentity(out res, id);
            if (res == "")
            {
                return Json(0);
            }
            List<object> p = new List<object>();
            string[] words = res.Replace(".","").Replace(",", "").Replace("\"", "").Replace("\'", "").Replace("(", "").Replace(")", "").Split(" ");
            res = res.ToUpper();
            List<string> words_2=new List<string>();

            foreach (var w in words)
            {
                if (w == "") continue;
                bool find = false;
                List<int> bid;
                su.FindAutoValues(out bid, w);
                String error="";
                foreach (int id1 in bid)
                    find |= su.FindSystem(ref p, id1, ref error);
                if (!find)
                {
                    words_2.Add(w.ToUpper());
                }
            }
            foreach (var o in p)
            {
                product p1 = o as product;
                double cost = 0;
                string a = p1.av.ToUpper();
                string[] avs = a.Split(",");
                foreach (var av in avs)
                    if (res.IndexOf(av) == -1)
                    {
                        if (av.IndexOf(" ") == -1)
                        {
                            double maxcost = 0;
                            foreach (var w in words_2)
                            {
                                double c = string_compare(av, w, out mask);
                                if (maxcost < c) maxcost = c;
                            }
                            cost += maxcost;
                        }
                        else
                        {
                            double maxcost = 0;
                            int delta = res.Length - av.Length;
                            if (delta < 0) delta = 0;
                            for (int i = 0; i <= delta; i++)
                            {
                                string w = res.Substring(i, av.Length);
                                double c = string_compare(av, w, out mask);
                                if (maxcost < c) maxcost = c;
                            }
                            cost += maxcost;
                        }
                    }
                    else
                    {
                        cost += 1;
                    }
                p1.cost = cost;
            }
            p.Sort((x, y) => -(x as product).cost.CompareTo((y as product).cost));

            if (deep > p.Count) deep = p.Count;
            product[] ps = new product[deep];

            for (int i = 0; i < deep; i++)
            {
                var o = p[i] as product;
                ps[i] = o;
            }
            return Json(ps);
        }
        public IActionResult Privacy()
        {
            return View();
        }
        [ResponseCache(Duration = 0, Location = ResponseCacheLocation.None, NoStore = true)]
        public IActionResult Error()
        {
            return View(new ErrorViewModel { RequestId = Activity.Current?.Id ?? HttpContext.TraceIdentifier });
        }
    }
}
