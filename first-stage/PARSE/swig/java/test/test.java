/*
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.  You may obtain
 * a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

import java.util.ArrayList;
import java.util.List;

public class test {
    static {
        System.loadLibrary("SWIGParser");
    }

    public static void main(String argv[]) {
        initialize(5);
        testReadAndParse();
        testMultiwordExtPos();
        testTokenizer();
        testMakeSentRep();
        testParse();
        testExtPos();

        /*
        for (int i = 0; i < 1000; i++) {
            System.out.println("iteration " + i);
            testTokenizer();
            testMakeSentRep();
            testParse();
            testExtPos();
        }
        */
        System.out.println("done");
    }

    public static void initialize(int nbest) {
        SWIGParser.loadModel("../DATA/EN");
        SWIGParser.setOptions("En", false, nbest, true, 21, 0, 0);
    }

    public static void testTokenizer() {
        SentRep sent = SWIGParser.tokenize("<s> Here's some text to tokenize. </s>", 399);
        dumpSentRep(sent);
    }

    public static void testMakeSentRep() {
        SentRep sent = makeSentRep(new String[] {"These", "are", "also", "tokens", "."});
        dumpSentRep(sent);
    }

    public static void testParse() {
        SentRep sent = makeSentRep(new String[] {"These", "are", "also", "tokens", "."});
        List<ScoredTreePair> parses = parse(sent);
        dumpParses(parses);
    }

    public static void testExtPos() {
        SentRep sent = makeSentRep(new String[] {"record"});

        System.out.println("Unconstrained");
        List<ScoredTreePair> parses = parse(sent);
        dumpParses(parses);

        ExtPos extPos1 = new ExtPos();
        VectorString vs1 = new VectorString();
        vs1.add("NN");
        extPos1.addTagConstraints(vs1);

        System.out.println("NN");
        parses = parse(sent, extPos1);
        dumpParses(parses);

        ExtPos extPos2 = new ExtPos();
        VectorString vs2 = new VectorString();
        vs2.add("VB");
        extPos2.addTagConstraints(vs2);

        System.out.println("VB");
        parses = parse(sent, extPos2);
        dumpParses(parses);
    }

    public static void testMultiwordExtPos() {
        SentRep sent = makeSentRep("British left waffles on Falklands .".split(" "));

        System.out.println("Unconstrained");
        List<ScoredTreePair> parses = parse(sent);
        dumpParses(parses);

        ExtPos extPos1 = makeExtPos(null,
                                    null,
                                    new String[] {"NNS"},
                                    null,
                                    null,
                                    null);

        System.out.println("NNS");
        parses = parse(sent, extPos1);
        dumpParses(parses);

        ExtPos extPos2 = makeExtPos(null,
                                    null,
                                    new String[] {"VBZ", "VBD", "VB"},
                                    null,
                                    null,
                                    null);

        System.out.println("VBZ/VBD/VB");
        parses = parse(sent, extPos2);
        dumpParses(parses);

        ExtPos extPos3 = makeExtPos(null,
                                    null,
                                    new String[] {"VBZ"},
                                    null,
                                    null,
                                    null);

        System.out.println("VBZ");
        parses = parse(sent, extPos3);
        dumpParses(parses);

        ExtPos extPos4 = makeExtPos(null,
                                    null,
                                    new String[] {"VBD"},
                                    null,
                                    null,
                                    null);

        System.out.println("VBD");
        parses = parse(sent, extPos4);
        dumpParses(parses);
    }

    public static void testReadAndParse() {
        InputTree tree = SWIGParser.inputTreeFromString("(S1 (S (NP (DT These)) (VP (AUX are) (RB also) (NP (VBZ tokens))) (. .)))");
        System.out.println("inputTreeFromString: " + tree);
        SentRep sent = tree.toSentRep();
        System.out.println("sent: " + sent);
        System.out.println("fail tree from sentence: " + sent.makeFailureTree("X"));
        dumpParses(parse(sent));
    }

    /*
     * Utility methods
     */

    public static SentRep makeSentRep(String[] tokens) {
        StringList stringList = new StringList();
        for (String token : tokens) {
            stringList.add(token);
        }
        return new SentRep(stringList);
    }

    public static void dumpSentRep(SentRep sentRep) {
        System.out.println("sentRep: |" + sentRep + "|");
        System.out.println("sentRep length: " + sentRep.length());
        for (int i = 0; i < sentRep.length(); i++) {
            System.out.println("sentRep token " + i + ": " +
                               sentRep.getWord(i).lexeme());
        }
    }

    public static List<ScoredTreePair> parse(SentRep sentRep) {
        return parse(sentRep, null);
    }

    public static List<ScoredTreePair> parse(SentRep sentRep, ExtPos extPos) {
        List<ScoredTreePair> results = new ArrayList<ScoredTreePair>();
        ScoreVector scoreList;
        if (extPos == null) {
            scoreList = SWIGParser.parse(sentRep);
        } else {
            if (sentRep.length() != extPos.size()) {
                throw new RuntimeException("ExtPos constraints don't match the length of the sentence (extPos: " + extPos.size() + ", sentence: " + sentRep.length() + ")");
            }
            scoreList = SWIGParser.parse(sentRep, extPos);
        }

        // ScoreVector isn't Iterable so we copy its contents over to a Java List
        for (int i = 0; i < scoreList.size(); i++) {
            results.add(scoreList.get(i));
        }

        return results;
    }

    public static void dumpParses(List<ScoredTreePair> parses) {
        int i = 0;
        for (ScoredTreePair scoredTreePair : parses) {
            System.out.println("Parse " + i + ":");
            InputTree tree = scoredTreePair.getSecond();
            System.out.println(scoredTreePair.getFirst() + "\n" + tree);
            System.out.println(tree.toStringPrettyPrint() + "\n");
            i++;
        }
    }

    public static ExtPos makeExtPos(String[]... possibleTagArray) {
        ExtPos extPos = new ExtPos();
        for (String[] possibleTags : possibleTagArray) {
            VectorString tagConstraints = new VectorString();
            if (possibleTags != null) {
                for (String tag : possibleTags) {
                    tagConstraints.add(tag);
                }
            }
            extPos.addTagConstraints(tagConstraints);
        }

        return extPos;
    }
}
