import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;


public class InitsbtdatServiceTest {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		/*
		ExecutorService es = Executors.newFixedThreadPool(2);
		es.execute(new Runnable() {
			
			@Override
			public void run() {
				for(int i = 0; i < 1000; i++) {
					try {
						Thread.sleep(2);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
					System.out.println(Thread.currentThread().getName() +  " for " + i);
				}
				
			}
		});
		
		es.execute(new Runnable() {
			
			@Override
			public void run() {
				int i = 0;
				while(true) {
					try {
						Thread.sleep(2);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
					System.out.println(Thread.currentThread().getName() +  " for " + i);
					i++;
				}
				
			}
		});
		*/
		int busyTime = 10;
		int idleTime = busyTime * 2;
		long startTime = 0;
		while(true) {
			startTime = System.currentTimeMillis();
			while((System.currentTimeMillis()-startTime) <= busyTime);
			try {
				Thread.sleep(idleTime);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}

}
