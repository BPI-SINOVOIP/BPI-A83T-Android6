package com.android.calculator2;

import java.io.File;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;

public class TestModeManager {
	public final static String TEST_MODE_KEY = "33";
	public final static String TEST_MODE_CONFIG = "23";
	private final static String FLAG_SDCARD = "/mnt/sdcard/DragonFire/custom_cases.xml";
	private final static String FLAG_AGING_SDCARD = "/mnt/sdcard/DragonFire/custom_cases_aging.xml";
	private final static String FLAG_SDCARD_CONFIG = "/mnt/sdcard/DragonFire/";

	public static boolean start(Context context, String inputKey) {
		if (inputKey.equals(TEST_MODE_CONFIG)) {
			return checkAndStartConfig(context, inputKey);
		} else if (inputKey.equals(TEST_MODE_KEY)) {
			return checkAndStart(context, inputKey);
		}
		return false;
	}

	private static boolean checkAndStart(Context context, String inputKey) {
		boolean b = false;
		File file = findDragonFire("/storage", "DragonFire/custom_cases.xml");
		File agingfile = findDragonFire("/storage", "DragonFire/custom_cases_aging.xml");
		if (file != null || agingfile != null || new File(FLAG_SDCARD).exists() || new File(FLAG_AGING_SDCARD).exists()) {
			Intent i = new Intent();
			ComponentName component = new ComponentName(
					"com.softwinner.dragonfire",
					"com.softwinner.dragonfire.SplashScreen_test");

			i.setComponent(component);
			try {
				context.startActivity(i);
				b = true;
			} catch (Exception e) {
			}
		}
		return b;
	}

	private static boolean checkAndStartConfig(Context context, String inputKey) {
		boolean b = false;
		File file = findDragonFire("/storage", "DragonFire");
		if (file != null || new File(FLAG_SDCARD_CONFIG).exists()) {
			Intent i = new Intent();
			ComponentName component = new ComponentName(
					"com.softwinner.dragonfire",
					"com.softwinner.dragonfire.SplashScreen_configuration");
			i.setComponent(component);
			try {
				context.startActivity(i);
				b = true;
			} catch (Exception e) {
			}
		}
		return b;
	}

	private static File findDragonFire(String dir, String path) {
		File file = new File(dir);
		File list[] = file.listFiles();
		for (File f : list) {
			if (f.isDirectory()) {
				File dfFile = new File(f, path);
				if (dfFile.exists())
					return dfFile;
			}
		}
		return null;
	}
}

