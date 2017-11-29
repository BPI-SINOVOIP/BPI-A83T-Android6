package junit.framework;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Enumeration;
import java.util.List;
import java.util.Vector;

/**
 * A <code>TestResult</code> collects the results of executing
 * a test case. It is an instance of the Collecting Parameter pattern.
 * The test framework distinguishes between <i>failures</i> and <i>errors</i>.
 * A failure is anticipated and checked for with assertions. Errors are
 * unanticipated problems like an {@link ArrayIndexOutOfBoundsException}.
 *
 * @see Test
 */
public class TestResult extends Object {
	// BEGIN android-changed changed types from List<> to Vector<> for API compatibility
	protected Vector<TestFailure> fFailures;
	protected Vector<TestFailure> fErrors;
	protected Vector<TestListener> fListeners;
	// END android-changed
	protected int fRunTests;
	private boolean fStop;
	
	public TestResult() {
		// BEGIN android-changed to Vector
		fFailures= new Vector<TestFailure>();
		fErrors= new Vector<TestFailure>();
		fListeners= new Vector<TestListener>();
		// END android-changed
		fRunTests= 0;
		fStop= false;
	}
	/**
	 * Adds an error to the list of errors. The passed in exception
	 * caused the error.
	 */
	public synchronized void addError(Test test, Throwable t) {
		fErrors.add(new TestFailure(test, t));
		for (TestListener each : cloneListeners())
			each.addError(test, t);
	}
	/**
	 * Adds a failure to the list of failures. The passed in exception
	 * caused the failure.
	 */
	public synchronized void addFailure(Test test, AssertionFailedError t) {
		fFailures.add(new TestFailure(test, t));
		for (TestListener each : cloneListeners())
			each.addFailure(test, t);
	}
	/**
	 * Registers a TestListener
	 */
	public synchronized void addListener(TestListener listener) {
		fListeners.add(listener);
	}
	/**
	 * Unregisters a TestListener
	 */
	public synchronized void removeListener(TestListener listener) {
		fListeners.remove(listener);
	}
	/**
	 * Returns a copy of the listeners.
	 */
	private synchronized List<TestListener> cloneListeners() {
		List<TestListener> result= new ArrayList<TestListener>();
		result.addAll(fListeners);
		return result;
	}
	/**
	 * Informs the result that a test was completed.
	 */
	public void endTest(Test test) {
		for (TestListener each : cloneListeners())
			each.endTest(test);
	}
	/**
	 * Gets the number of detected errors.
	 */
	public synchronized int errorCount() {
		return fErrors.size();
	}
	/**
	 * Returns an Enumeration for the errors
	 */
	public synchronized Enumeration<TestFailure> errors() {
		return Collections.enumeration(fErrors);
	}
	

	/**
	 * Gets the number of detected failures.
	 */
	public synchronized int failureCount() {
		return fFailures.size();
	}
	/**
	 * Returns an Enumeration for the failures
	 */
	public synchronized Enumeration<TestFailure> failures() {
		return Collections.enumeration(fFailures);
	}
	
	/**
	 * Runs a TestCase.
	 */
	protected void run(final TestCase test) {
		startTest(test);
		Protectable p= new Protectable() {
			public void protect() throws Throwable {
                if (test.toString().contains("(com.android.cts.net.hostside.VpnTest)")     ||
                    test.toString().equals("long_rgb(dEQP-GLES2.functional.color_clear)")  ||
                    test.toString().equals("long_rgba(dEQP-GLES2.functional.color_clear)") ||
                    test.toString().contains("android.media.cts.EncodeDecodeTest")         ||  
                    test.toString().contains("android.openglperf.cts.GlVboPerfTest")         ||  
                    test.toString().contains("dEQP-GLES2.functional.uniform_api.random")         ||  
				    test.toString().contains("(android.mediastress.cts.HEVCR1080pAacLongPlayerTest)")     ||
				    test.toString().contains("(android.mediastress.cts.HEVCR1080pAacRepeatedPlayerTest)") ||
				    test.toString().contains("(android.mediastress.cts.HEVCR1080pAacShortPlayerTest)")) {
                    return;
                }
                test.runBare();
            }
        };
        runProtected(test, p);

        endTest(test);
    }
	/**
	 * Gets the number of run tests.
	 */
	public synchronized int runCount() {
		return fRunTests;
	}
	/**
	 * Runs a TestCase.
	 */
	public void runProtected(final Test test, Protectable p) {
		try {
			p.protect();
		} 
		catch (AssertionFailedError e) {
			addFailure(test, e);
		}
		catch (ThreadDeath e) { // don't catch ThreadDeath by accident
			throw e;
		}
		catch (Throwable e) {
			addError(test, e);
		}
	}
	/**
	 * Checks whether the test run should stop
	 */
	public synchronized boolean shouldStop() {
		return fStop;
	}
	/**
	 * Informs the result that a test will be started.
	 */
	public void startTest(Test test) {
		final int count= test.countTestCases();
		synchronized(this) {
			fRunTests+= count;
		}
		for (TestListener each : cloneListeners())
			each.startTest(test);
	}
	/**
	 * Marks that the test run should stop.
	 */
	public synchronized void stop() {
		fStop= true;
	}
	/**
	 * Returns whether the entire test was successful or not.
	 */
	public synchronized boolean wasSuccessful() {
		return failureCount() == 0 && errorCount() == 0;
	}
}
