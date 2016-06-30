import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map.Entry;
import java.util.StringTokenizer;

class Task {
	String doc;

	Task(String doc) {
		this.doc = doc;
	}

	public String toString() {
		return doc;
	}
}

class Map extends Task {
	long offset, size;
	int number;

	Map(String doc, long offset, long size, int number) {
		super(doc);
		this.offset = offset;
		this.size = size;
		this.number = number;
	}

	public String toString() {
		return doc + " " + number;
	}

}

class Reduce extends Task {
	public ArrayList<String> maxWords;

	Reduce(String doc) {
		super(doc);
		maxWords = new ArrayList<String>();
	}
}

class Workers extends Thread {
	WorkPool wp;
	int id;
	final static String token = " ;:/?~\\.,><~`[]{}()!@#$%^&-_+\'=*\"|\t\n\r\0";

	// number of documents, split size
	Workers(WorkPool wp, int id) {
		this.wp = wp;
		this.id = id;
	}

	void resolveMap(Task task) {
		int index = 0, max = 0;
		char[] cbuf = new char[(int) ((Map) task).size];
		char[] x = new char[1];

			for (int i = 0; i < ReplicateWorkers.ND; i++)
				if (ReplicateWorkers.docName[i].equals(task.doc)) {
					index = i;
					break;
				}

		try {
			int auxOffset = 0;
			StringTokenizer splitter;
			String aux;
			BufferedReader br = new BufferedReader(
					new InputStreamReader(new FileInputStream(new File(task.doc)), "UTF-8"));
			// skip offset
			if (((Map) task).offset != 0) {
				br.skip(((Map) task).offset - 1);
				br.read(x);
				aux = String.valueOf(x);
				splitter = new StringTokenizer(aux, token);
				// stepped on FIRST word
				while (splitter.hasMoreTokens()) {
					br.read(x);
					aux = String.valueOf(x);
					splitter = new StringTokenizer(aux, token);
					// check for additional chars to be skipped
					if (splitter.hasMoreTokens())
						auxOffset++;
					if (auxOffset != 0 && !splitter.hasMoreTokens()) {
						auxOffset++;
					}
				}
			}
			// read chunk
			br.read(cbuf, 0, (int) (((Map) task).size - auxOffset));
			String str = new String(cbuf, 0, (int) (((Map) task).size - auxOffset));
			x[0] = cbuf[cbuf.length - 1 - auxOffset];
			// stepped on LAST word
			aux = String.valueOf(x);
			splitter = new StringTokenizer(aux, token);
			if (((Map) task).offset + ((Map) task).size < (new File(task.doc)).length()) {
				while (splitter.hasMoreTokens()) {
					br.read(x);
					aux = String.valueOf(x);
					splitter = new StringTokenizer(aux, token);
					// check for additional chars to be added as last word
					if (splitter.hasMoreTokens()) {
						str += x[0];
					}
				}
			}

			// split it
			splitter = new StringTokenizer(str, token);
			// parse it and add into hashmap and list of maximum words
			while (splitter.hasMoreTokens()) {
				String word = splitter.nextToken();
				int value = 0;
				synchronized (ReplicateWorkers.listOfMaps) {
					if (ReplicateWorkers.listOfMaps.get(index).containsKey(word.length()))
						value = ReplicateWorkers.listOfMaps.get(index).get(word.length());
					// put element in hashmap
					ReplicateWorkers.listOfMaps.get(index).put(word.length(), value + 1);
					ReplicateWorkers.totalWords[index]++;
				}
				// add max word to list
				if (max == word.length()) {
					ReplicateWorkers.listOfMax.get(index).get(((Map) task).number).add(word);
				} else if (max < word.length()) {
					max = word.length();
					// clear curent list, a new max has been found
					ReplicateWorkers.listOfMax.get(index).get(((Map) task).number).clear();
					ReplicateWorkers.listOfMax.get(index).get(((Map) task).number).add(word);
				}

			}

			br.close();
		} catch (UnsupportedEncodingException e) {
			e.printStackTrace();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}

		synchronized (ReplicateWorkers.parts) {
			// check if all map tasks finished for a document
			ReplicateWorkers.parts[index]--;
			if (ReplicateWorkers.parts[index] == 0) {
				Reduce r = new Reduce(task.doc);
				// put reduce to work
				wp.putWork(r);
				ReplicateWorkers.parts[index]--;
			}
		}
	}

	int doFibo(int n) {
		int a = 0;
		int b = 1;
		int c = 1;
		for (int i = 1; i < n; i++) {
			c = a + b;
			a = b;
			b = c;
		}
		return c;
	}

	void resolveReduce(Task task) {
		
			int index = 0;
			// find doc name index
			for (int i = 0; i < ReplicateWorkers.ND; i++)
				if (ReplicateWorkers.docName[i].equals(task.doc)) {
					index = i;
					break;
				}
			// COMBINE
			int maxDim = 0;
			for (ArrayList<String> arr : ReplicateWorkers.listOfMax.get(index)) {
				if (!arr.isEmpty()) {
					// a maximum size has been discovered
					if (arr.get(0).length() > maxDim) {
						maxDim = arr.get(0).length();
						// clear saved words, and size
						((Reduce) task).maxWords.clear();
						ReplicateWorkers.maxWords[index] = 0;
						for (String toBeAdded : arr) {
							// add new words and size
							if (!((Reduce) task).maxWords.contains(toBeAdded.toLowerCase())) {
								((Reduce) task).maxWords.add(toBeAdded.toLowerCase());
								ReplicateWorkers.maxWords[index]++;
							}
						}
					}
					// on equal sizes just count new words
					if (arr.get(0).length() == maxDim) {
						for (String toBeAdded : arr)
							if (!((Reduce) task).maxWords.contains(toBeAdded.toLowerCase())) {
								((Reduce) task).maxWords.add(toBeAdded.toLowerCase());
								ReplicateWorkers.maxWords[index]++;
							}
					}
				}
				// set max length 
				ReplicateWorkers.maxLen[index] = maxDim;
			}
			
			// PROCESS
			synchronized (ReplicateWorkers.rank) {
			for (Entry<Integer, Integer> entry : ReplicateWorkers.listOfMaps.get(index).entrySet())
				ReplicateWorkers.rank[index] += (doFibo(entry.getKey() + 1) * entry.getValue());

			ReplicateWorkers.rank[index] /= ReplicateWorkers.totalWords[index];
			// truncate
			ReplicateWorkers.rank[index] = Math.floor(ReplicateWorkers.rank[index] * 100) / 100;
			}
		
	}

	/**
	 * Thread job
	 */
	public void run() {
		while (true) {
			// check for new tasks
			Task task = wp.getWork();
			if (task == null) {
				if (id == 0) {
					ReplicateWorkers.writeOutput();
				}
				break;
			}
			// Map
			if (task instanceof Map) {
				resolveMap(task);
			}
			// Reduce
			if (task instanceof Reduce) {
				resolveReduce(task);
			}
		}
	}
}

public class ReplicateWorkers {
	public static int D, ND, parts[], maxLen[], maxWords[], totalWords[];
	public static double rank[];
	public static String docName[], docNameAux[];
	public static List<HashMap<Integer, Integer>> listOfMaps;
	public static List<ArrayList<ArrayList<String>>> listOfMax;
	public static String output,input;
	
	public static void main(String[] args) {
		input = args[1];
		output = args[2];
		readInput();
		// TREBUIE CA PARAMETRU NUMBER OF THREADS
		int numThreads = Integer.parseInt(args[0]);
	
		// initialize workpool with number of threads
		WorkPool wp = new WorkPool(numThreads);
		Map task;
		File f;
		int j;
		long size, fragments, total;
		// add MAP tasks
		for (int i = 0; i < ND; i++) {
			f = new File(docName[i]);
			size = f.length();
			fragments = size / D;
			total = 0;
			// split tasks
			for (j = 0; j < fragments; j++) {
				total += D;
				parts[i]++;
				task = new Map(docName[i], j * D, D, j);
				listOfMax.get(i).add(new ArrayList<String>());
				wp.putWork(task);
			}
			// last fragment has variable size
			if (total < size) {
				parts[i]++;
				task = new Map(docName[i], total, size - total, j);
				listOfMax.get(i).add(new ArrayList<String>());
				wp.putWork(task);
			}
		}
		// start getting tasks
		Workers w[] = new Workers[numThreads];
		for (int i = 0; i < numThreads; i++) {
			w[i] = new Workers(wp, i);
			w[i].start();
		}

	}

	static void readInput() {
		// TREBUIE CA PARAMETRU INPUT
		BufferedReader br;
		try {
			br = new BufferedReader(new FileReader(new File(input)));

			D = Integer.parseInt(br.readLine());
			ND = Integer.parseInt(br.readLine());
			parts = new int[ND];
			docName = new String[ND];
			docNameAux = new String[ND];
			rank = new double[ND];
			maxLen = new int[ND];
			maxWords = new int[ND];
			totalWords = new int[ND];
			listOfMaps = new ArrayList<HashMap<Integer, Integer>>(ND);
			listOfMax = new ArrayList<ArrayList<ArrayList<String>>>(ND);
			for (int i = 0; i < ND; i++) {
				docName[i] = br.readLine();
				docNameAux[i] = docName[i];
				listOfMaps.add(new HashMap<Integer, Integer>());
				listOfMax.add(new ArrayList<ArrayList<String>>());
			}
			br.close();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}

	}

	static void writeOutput() {
		try {
			int sw = 0;
			double auxd = 0;
			int auxi = 0;
			String auxs = null;
			// TREBUIE CA PARAMETRU OUTPUT
			BufferedWriter bw = new BufferedWriter(new FileWriter(new File(output)));

			// sort it
			do {
				sw = 0;
				for (int i = 0; i < ND - 1; i++)
					if (rank[i] < rank[i + 1]) {
						sw = 1;
						// ranks
						auxd = rank[i];
						rank[i] = rank[i + 1];
						rank[i + 1] = auxd;
						// names
						auxs = docName[i];
						docName[i] = docName[i + 1];
						docName[i + 1] = auxs;
						// maxLen
						auxi = maxLen[i];
						maxLen[i] = maxLen[i + 1];
						maxLen[i + 1] = auxi;
						// maxWords
						auxi = maxWords[i];
						maxWords[i] = maxWords[i + 1];
						maxWords[i + 1] = auxi;
					}
					// rank is equal
					else if (rank[i] == rank[i + 1]) {
						int idx1 = 0, idx2 = 0;
						// find doc name index
						for (int j = 0; j < ReplicateWorkers.ND; j++) {
							if (docNameAux[j].equals(docName[i])) {
								idx1 = j;
							}
							if (docNameAux[j].equals(docName[i + 1])) {
								idx2 = j;
							}
						}
						if (idx2 < idx1) {
							sw = 1;
							// ranks
							auxd = rank[i];
							rank[i] = rank[i + 1];
							rank[i + 1] = auxd;
							// names
							auxs = docName[i];
							docName[i] = docName[i + 1];
							docName[i + 1] = auxs;
							// maxLen
							auxi = maxLen[i];
							maxLen[i] = maxLen[i + 1];
							maxLen[i + 1] = auxi;
							// maxWords
							auxi = maxWords[i];
							maxWords[i] = maxWords[i + 1];
							maxWords[i + 1] = auxi;
						}
					}
			} while (sw == 1);
			// write in file
			for (int i = 0; i < ND; i++) {
				String str ="";
				str = docName[i] + ";" + String.format("%.2f", rank[i])+ ";[" + maxLen[i] + "," + maxWords[i] + "]\n";
				bw.write(str);
			}

			bw.close();
		} catch (IOException e) {
			e.printStackTrace();
		}

	}
}
