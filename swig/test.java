public class test {

  public static void main(String argv[]) {
    System.loadLibrary("exodus");
    var c=new var();
    System.out.println(exodusJNI.mvtypemask_get());
    System.out.println("done");
  }
}
