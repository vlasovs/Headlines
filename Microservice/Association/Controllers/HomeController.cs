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
        static double string_compare(string s1, string s2)
        {
            int MATCH = 0;
            int INSERT = 1;
            int DELETE = 2;
            int[] opt = { 0, 0, 0 };
            int[,] cost = new int[s1.Length + 1, s2.Length + 1];

            for (int i = 0; i < s1.Length + 1; i++)
            {
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
            return Math.Exp(-cost[s1.Length, s2.Length]);
        }

        public JsonResult Index(int id=-1, int deep = 10)
        {
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
            res = res.ToUpper();
            List<object> p = new List<object>();
            string[] words = res.Split(" ");
            foreach (var w in words)
            {
                List<int> bid;
                su.FindAutoValues(out bid, w);
                foreach (int id1 in bid)
                    su.FindSystem(ref p, id1);
            }
            foreach (var o in p)
            {
                product p1 = o as product;
                double cost = 0;
                if (res.IndexOf(p1.av.ToUpper()) == -1)
                {
                    double maxcost = 0;
                    foreach (var w in words)
                    {
                        double c = string_compare(p1.av, w);
                        if (maxcost < c) maxcost = c;
                    }
                    cost = maxcost;
                }
                else
                {
                    cost = 1;
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
