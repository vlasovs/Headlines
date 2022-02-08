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
            }
            conn.Close();
            return Error;
        }
        
        public String FindSystem(ref List<object> p, int bid)
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
            q.Parameters.Add("@bid", MySqlDbType.String).Value = bid;

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
                            product p1 = new product();
                            p1.id = reader.GetInt32(0);
                            p1.av = reader.GetString(1);
                            p1.cost = 0;
                            if (p1.av != "")
                            {
                                p.Add(p1);
                            }
                        }
                    }
                }
            }
            catch (Exception exc)
            {
                Error = "ReadFromDB " + q.CommandText + " - Error: " + exc.Message;
            }
            conn.Close();
            return Error;
        }
    }
}
