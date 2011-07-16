
import jexodus.*;

public class test {

public static void main(String argv[]) {
	System.out.println("--- Testing ---");

	System.loadLibrary("jexodus");
	var exo=new var("");

	var filename=new var("tempdbfilejava");
	var options=new var("");

	if (!exo.connect(options)) {
        	System.out.println("no default database connection");
	} else {

	        filename=new var("tempdbfile564");
        	options=new var("");

	        if (exo.createfile("tempdbfile564")) {
        	        System.out.println("filename file created");
        	}else{
                	System.out.println("filename file not created. maybe already exists. try deletefile filename");
        	}

		exo.begintrans();
        	//write some records
                var record=new var("X").str(1000);
        	for (int ii=1;ii<=100;++ii) {
                	var id=new var(ii);
                	if (!record.write(filename,id)) {
                        	break;
	                }
        	        System.out.print(ii);
        	        System.out.print(" ");
        	}
        	System.out.println();
		exo.committrans();

	        if (exo.deletefile(filename)) {
         	       System.out.println("filename file deleted");
        	}
	}

	System.out.println(exo.listfiles());

	var D=new var("D");
	var MT=new var("MT");
	System.out.println("Pick Date: "+exo.date()+" is "+exo.date().oconv(D));
	System.out.println("Pick Time: "+exo.time()+" is "+exo.time().oconv(MT));

	System.out.println("Finished");
}
}

