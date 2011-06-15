
import jexodus.*;

public class test {

public static void main(String argv[]) {
	System.out.println("Started");

	System.loadLibrary("jexodus");
	var exo=new var("");

	var filename=new var("tempdbfilejava");
	var options=new var("");
	if (exo.createfile(filename,options)) {
		if (filename.deletefile())
			System.out.println("File deleted");
	}
	else
		System.out.println("File not created");

	System.out.print(exo.listfiles());

	System.out.println("Finished");
}
}

