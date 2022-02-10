using Microsoft.AspNetCore.Mvc;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Tutorial.MySiteUtils;

namespace Association.Controllers
{
    public class BrandController : Controller
    {
        public JsonResult Index(int id = -1)
        {
            try
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
                List<object> p = new List<object>();
                string[] words = res.Replace(".", "").Replace(",", "").Replace("\"", "").Replace("\'", "").Replace("(", "").Replace(")", "").Split(" ");
                res = res.ToUpper();
                List<string> words_2 = new List<string>();
                List<int> bids=new List<int>();
                foreach (var w in words)
                {
                    if (w == "") continue;
                    List<int> bid;
                    su.FindAutoValues(out bid, w);
                    bids.AddRange(bid);
                }
                return Json(bids);
            }
            catch (Exception e)
            {
                return Json(e.Message);
            }
        }
    }
}
