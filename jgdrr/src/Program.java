import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;

import rnati.*;
import rreil.DefaultRReilBuilder;
import rreil.IRReilCollection;

public class Program {
	/**
	 * @param args
	 * @throws IOException
	 */
	public static void main(String[] args) throws IOException {
		BufferedReader reader = new BufferedReader(new InputStreamReader(
				System.in));
		while (true) {
			String line = reader.readLine();
			if(line == null)
				break;
			else if(line.length() == 0)
				continue;				
			
			System.out.println("Interpreting \"" + line + "\"...");

			ArrayList<Byte> byteList = new ArrayList<Byte>();

			byte next = 0;
			int part = 0;
			for (int i = 0; i < line.length(); i++) {
				char nextChar = line.charAt(i);
				if (nextChar == 'x') {
					part = next = 0;
					continue;
				} else if (nextChar == ' ')
					continue;
				else if (nextChar >= 'a' && nextChar <= 'f')
					nextChar = (char) ((nextChar - 'a') + 10);
				else if (nextChar >= '0' && nextChar <= '9')
					nextChar -= '0';
				else
					throw new RuntimeException("Invalid input.");
				if (part == 0)
					next = (byte) (nextChar << 4);
				else {
					next |= nextChar;
					byteList.add(next);
				}
				part = 1 - part;
			}

			byte[] bytes = new byte[byteList.size()];
			for (int i = 0; i < bytes.length; i++)
				bytes[i] = byteList.get(i);

			DefaultRReilBuilder builder = new DefaultRReilBuilder();
			NativeInterface n = new NativeInterface(builder);

			System.out.println("Decoding and translating...");

			IRReilCollection c = n.decodeAndTranslate(bytes);

			// IRReilCollection c = n.decodeAndTranslate(new byte[] { 0x66,
			// 0x0f,
			// 0x38, 0x04, (byte) 0xd1 });
			// IRReilCollection c = n.decodeAndTranslate(new byte[] { 0x66 });
			// IRReilCollection c = n.decodeAndTranslate(null);

			System.out.println("RReil:");

			for (int i = 0; i < c.size(); i++) {
				System.out.println(c.get(i));
			}
		}
	}
}
