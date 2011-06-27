using System;

 public class test {
     static void Main() {

        Console.WriteLine("--- Testing ---");

//in C#2008 onwards you can put var xyz=new ...
//csharp has no way to say "for var put mvar" for old versions that dont support it. "welcome to the machine, my son"
//        var exo=new mvar("");

        mvar exo=new mvar("");

//"cannot explicitly convert "string" to mvar (yet!)
//      mvar xyz="xyz";

        string str1="MT";

        mvar testnum=new mvar(1000);
        Console.WriteLine(testnum);
        Console.WriteLine(testnum.toInt()+1);
        Console.WriteLine(testnum.quote());

        mvar filename=new mvar("T");
        Console.WriteLine(filename);

        Console.WriteLine(filename.quote());
        mvar options=new mvar("");

        if (!exo.connect(options))
                Console.WriteLine("couldnt connect to default database");
        else {
                Console.WriteLine("connected to default database");

	        if (exo.open(filename,options))
	                Console.WriteLine("db file already existed.");

		else if (exo.createfile(filename))
			Console.WriteLine("db file created");
		else
			Console.WriteLine("db file not created");

	        //write some records
		exo.begintrans();
		mvar record=(new mvar("X")).str(1000);
	        for (int ii=1;ii<=100;++ii) {
	                mvar id=new mvar(ii);
	                if (!record.write(filename,id))
	                        break;
	                Console.Write(id+" ");
	        }
	        Console.WriteLine();
		exo.committrans();

	        mvar listoffiles=exo.listfiles();
	        Console.WriteLine(listoffiles);
	        Console.WriteLine(listoffiles.extract(1));

	        if (filename.deletefile())
	                Console.WriteLine("db file deleted");

	}

        //mvar MT=new mvar("MT");
        string MTs="MT";
        mvar MT=new mvar("MT");
        Console.WriteLine("Pick Date: "+exo.date()+" is "+exo.date().oconv("D"));
        Console.WriteLine("Pick Time: "+exo.time()+" is "+exo.time().oconv(MT));
MT.seq().outputl();
        Console.WriteLine("Pick Time: "+exo.time()+" is "+exo.time().oconv(MTs));

        Console.WriteLine("Finished");

     }
 }


