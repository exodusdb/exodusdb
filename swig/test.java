
import jexodus.*;

public class test {

public static void main(String argv[]) {
	System.out.println("Started");

	System.loadLibrary("jexodus");
	var exo=new var("");

	var filename=new var("tempdbfilejava");
	var options=new var("");

	if (!exo.connect(options))
		System.out.println("couldnt connect to default database");

	if (exo.open(filename,options))
		System.out.println("db file already existed.");		
	else if (exo.createfile(filename,options)) {
		System.out.println("db file created");
		if (filename.deletefile())
			System.out.println("db file deleted");
	} else
		System.out.println("db file not created");
	if (filename.deletefile())
		System.out.println("db file deleted");

	System.out.println(exo.listfiles());

	var D=new var("D");
	var MT=new var("MT");
	System.out.println("Pick Date: "+exo.date()+" is "+exo.date().oconv(D));
	System.out.println("Pick Time: "+exo.time()+" is "+exo.time().oconv(MT));

	System.out.println("Finished");
}
}

