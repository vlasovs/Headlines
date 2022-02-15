using System;
using System.Collections.Generic;
using MySql.Data.MySqlClient;
using Tutorial.SqlConn;
using System.Data.Common;

namespace Tutorial.MySiteUtils
{
    public class product
    {
        public int id { get; set; }
        public string av { get; set; }
        public double cost { get; set; }

    }
    class SiteUtils
    {
        public String ReadSiteIdentity(out string res, int Id)
        {
            res = "";

            string host = "db.z-price";
            int port = 3306;
            string database = "sites";
            string username = "dev";
            string password = "sadfa324easd";
            string characterset = "utf8";

            MySqlConnection conn = DBMySQLUtils.GetDBConnection(host, port, database, username, password, characterset);
            MySqlCommand q = conn.CreateCommand();

            q.CommandText = "SELECT SiteIdentity FROM urls_2 WHERE Id=@Id";
            q.Parameters.Add("@Id", MySqlDbType.Int32).Value = Id;

            String Error = "";
            try
            {
                conn.Open();
                //q.Connection = conn;
                using (DbDataReader reader = q.ExecuteReader())
                {
                    if (reader.HasRows)
                    {
                        while (reader.Read())
                        {
                            res = reader.GetString(0);
                        }
                    }
                }
            }
            catch (Exception exc)
            {
                Error = "ReadFromDB " + q.CommandText + " - Error: " + exc.Message;
                throw (new Exception(Error));
            }
            conn.Close();
            return Error;
        }

        public String FindAutoValues(out List<int> Id, string av)
        {
            string host = "db.z-price";
            int port = 3306;
            string database = "z-price";
            string username = "dev";
            string password = "sadfa324easd";
            string characterset = "utf8";

            MySqlConnection conn = DBMySQLUtils.GetDBConnection(host, port, database, username, password, characterset);
            MySqlCommand q = conn.CreateCommand();

            Id = new List<int>();

            q.CommandText = "SELECT ID FROM brands WHERE AutoValues LIKE CONCAT('%',@AV,'%');";
            q.Parameters.Add("@AV", MySqlDbType.String).Value = av;

            String Error = "";
            try
            {
                conn.Open();
                //q.Connection = conn;
                using (DbDataReader reader = q.ExecuteReader())
                {
                    if (reader.HasRows)
                    {
                        while (reader.Read())
                        {
                            Id.Add(reader.GetInt32(0));
                        }
                    }
                }
            }
            catch (Exception exc)
            {
                Error = "ReadFromDB " + q.CommandText + " - Error: " + exc.Message;
                throw (new Exception(Error));
            }
            conn.Close();
            return Error;
        }

        public bool FindSystem(ref List<object> p, int bid, ref String error)
        {
            string host = "db.z-price";
            int port = 3306;
            string database = "z-price";
            string username = "dev";
            string password = "sadfa324easd";
            string characterset = "utf8";

            MySqlConnection conn = DBMySQLUtils.GetDBConnection(host, port, database, username, password, characterset);
            MySqlCommand q = conn.CreateCommand();

            q.CommandText = "SELECT ProductID,AV FROM products WHERE BrandId=@bid;";
            q.Parameters.Add("@bid", MySqlDbType.Int32).Value = bid;

            bool find = false;

            error = "";
            try
            {
                conn.Open();
                //q.Connection = conn;
                using (DbDataReader reader = q.ExecuteReader())
                {
                    if (reader.HasRows)
                    {
                        while (reader.Read())
                        {
                            product p1 = new product();
                            p1.id = reader.GetInt32(0);
                            p1.av = reader.GetString(1);
                            p1.cost = 0;
                            if (p1.av != "")
                            {
                                p.Add(p1);
                            }
                            find = true;
                        }
                    }
                }
            }
            catch (Exception exc)
            {
                error = "ReadFromDB " + q.CommandText + " - Error: " + exc.Message;
                throw (new Exception(error));
            }
            conn.Close();
            return find;
        }       

        public bool FindDuplicate(ref List<int> p, int id, ref String error)
        {
            string host = "db.z-price";
            int port = 3306;
            string database = "z-price";
            string username = "dev";
            string password = "sadfa324easd";
            string characterset = "utf8";
            bool find = false;
            error = "";
            try
            {                
                MySqlConnection conn = DBMySQLUtils.GetDBConnection(host, port, database, username, password, characterset);
                conn.Open();
                MySqlCommand q = conn.CreateCommand();
                q.CommandText = "SELECT `AV` FROM products WHERE `ProductID`=@id;";
                q.Parameters.Add("@id", MySqlDbType.Int32).Value = id;
                string avs = "";
                using (DbDataReader reader = q.ExecuteReader())
                {
                    if (reader.HasRows)
                    {
                        while (reader.Read())
                        {
                            avs = reader.GetString(0);                            
                        }
                    }
                }
                SortedSet<int> pids = new SortedSet<int>();
                var s = avs.Split("\n");
                foreach (var av in s)
                {
                    if (av == "") continue;
                    MySqlCommand q2 = conn.CreateCommand();
                    q2.CommandText = "SELECT `ProductID` FROM products WHERE `AV` LIKE CONCAT('%', @av, '%');";
                    q2.Parameters.Add("@av", MySqlDbType.String).Value = av;
                    using (DbDataReader reader = q2.ExecuteReader())
                    {
                        if (reader.HasRows)
                        {
                            while (reader.Read())
                            {
                                int pid = reader.GetInt32(0);
                                if (pid != id)
                                {
                                    pids.Add(pid);
                                    find = true;
                                }
                            }
                        }
                    }
                }
                conn.Close();
                foreach (int i in pids) {
                    p.Add(i);
                }
            }
            catch (Exception exc)
            {
                error = "Error: " + exc.Message;
                throw (new Exception(error));                
            }            
            return find;
        }
    }
}