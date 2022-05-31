import java.util.ArrayList;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

public class Main6 {
    static public class FastQ {
        String name;
        String seq;
        String qual;
        public FastQ(String n, String s, String q){
            name = n;
            seq = s;
            qual = q;
        }
    }
    public static void main(String[] argv){
        ArrayList<FastQ> list = new ArrayList<>();
        try  {
            BufferedReader br = new BufferedReader(new FileReader(argv[0]));
            while(true){
                String name = br.readLine();
                String seq = br.readLine();
                br.readLine();
                String qual = br.readLine();
                if(name == null){
                    break;
                }
                FastQ fq = new FastQ(name, seq, qual);
                list.add(fq);
            }
            for(FastQ fq: list){
                System.out.println(fq.name + "\n" + fq.seq + "\n+\n" + fq.qual + "\n");
            }
        }catch(Exception e){
            e.printStackTrace();
        }
    }
}
