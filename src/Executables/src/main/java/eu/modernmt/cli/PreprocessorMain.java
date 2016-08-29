package eu.modernmt.cli;

import eu.modernmt.cli.init.Submodules;
import eu.modernmt.constants.Const;
import eu.modernmt.io.LineReader;
import eu.modernmt.io.LineWriter;
import eu.modernmt.io.UnixLineReader;
import eu.modernmt.io.UnixLineWriter;
import eu.modernmt.model.Sentence;
import eu.modernmt.processing.Preprocessor;
import eu.modernmt.processing.framework.PipelineInputStream;
import eu.modernmt.processing.framework.PipelineOutputStream;
import eu.modernmt.processing.framework.ProcessingException;
import eu.modernmt.processing.util.SentenceOutputter;
import eu.modernmt.processing.util.TokensOutputter;
import org.apache.commons.cli.*;
import org.apache.commons.io.IOUtils;

import java.util.Locale;

/**
 * Created by davide on 17/12/15.
 */
public class PreprocessorMain {

    static {
        Submodules.link();
    }

    private static class Args {

        private static final Options cliOptions;

        static {
            Option lang = Option.builder().longOpt("lang").hasArg().required().build();
            Option skipTags = Option.builder().longOpt("no-tags").hasArg(false).required(false).build();
            Option skipPlaceholders = Option.builder().longOpt("print-placeholders").hasArg(false).required(false).build();
            Option keepSpaces = Option.builder().longOpt("original-spacing").hasArg(false).required(false).build();

            cliOptions = new Options();
            cliOptions.addOption(lang);
            cliOptions.addOption(skipTags);
            cliOptions.addOption(skipPlaceholders);
            cliOptions.addOption(keepSpaces);
        }

        public final Locale language;
        public final boolean printTags;
        public final boolean printPlaceholders;
        public final boolean keepSpaces;

        public Args(String[] args) throws ParseException {
            CommandLineParser parser = new DefaultParser();
            CommandLine cli = parser.parse(cliOptions, args);

            language = Locale.forLanguageTag(cli.getOptionValue("lang"));
            printTags = !cli.hasOption("no-tags");
            printPlaceholders = cli.hasOption("print-placeholders");
            keepSpaces = cli.hasOption("original-spacing");
        }

    }

    public static void main(String[] _args) throws InterruptedException, ProcessingException, ParseException {
        Args args = new Args(_args);

        Preprocessor preprocessor = null;
        PipelineInputStream<String> input = null;
        PipelineOutputStream<Sentence> output = null;

        LineReader sin = new UnixLineReader(System.in, Const.charset.get());
        LineWriter sout = new UnixLineWriter(System.out, Const.charset.get());

        try {
            preprocessor = new Preprocessor(args.language);

            input = PipelineInputStream.fromLineReader(sin);
            if (args.keepSpaces)
                output = new SentenceOutputter(sout, args.printTags, args.printPlaceholders);
            else
                output = new TokensOutputter(sout, args.printTags, args.printPlaceholders);

            preprocessor.process(input, output);
        } finally {
            IOUtils.closeQuietly(preprocessor);
            IOUtils.closeQuietly(input);
            IOUtils.closeQuietly(output);
        }
    }

}
