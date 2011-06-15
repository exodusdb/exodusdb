
import org.exodus.*;

public class test {

public static void main(String argv[]) {
	System.out.println("Started");

	System.loadLibrary("exodus");
	var exo=new var("");

	var filename=new var("tempdbfilejava");
	var options=new var("");
	if (exo.createfile(filename,options)) {
		System.out.println("db file created");
		if (filename.deletefile())
			System.out.println("db file deleted");
	}
	else
		System.out.println("db file not created");

	if (filename.deletefile())
		System.out.println("db file deleted");

	System.out.println(exo.listfiles());

	System.out.println("Finished");
}
}

