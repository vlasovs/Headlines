using Microsoft.AspNetCore.Mvc;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Tutorial.MySiteUtils;

namespace Association.Controllers
{
    public class DuplicateController : Controller
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
                List<int> res=new List<int>();
                string error="";
                su.FindDuplicate(ref res, id, ref error);                
                return Json(res);
            }
            catch (Exception e)
            {
                return Json(e.Message);
            }
        }       
    }
}
