/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
 *     Guido Tack, 2010
 *
 *  Last modified:
 *     $Date: 2011-07-13 18:56:36 +0200 (Wed, 13 Jul 2011) $ by $Author: tack $
 *     $Revision: 12195 $
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <gecode/int/cumulative.hh>

#include <algorithm>

namespace Gecode {

  template<class Cap>
  void
  cumulative(Home home, Cap c, const TaskTypeArgs& t,
             const IntVarArgs& s, const IntArgs& p, const IntArgs& u, 
             IntConLevel icl) {
    using namespace Gecode::Int;
    using namespace Gecode::Int::Cumulative;
    if ((s.size() != p.size()) || (s.size() != u.size()) ||
        (s.size() != t.size()))
      throw Int::ArgumentSizeMismatch("Int::cumulative");
    double w = 0.0;
    for (int i=p.size(); i--; ) {
      Int::Limits::nonnegative(p[i],"Int::cumulative");
      Int::Limits::nonnegative(u[i],"Int::cumulative");
      Int::Limits::check(static_cast<double>(s[i].max()) + p[i],
                         "Int::cumulative");
      Int::Limits::double_check(static_cast<double>(p[i]) * u[i],
                                "Int::cumulative");
      w += s[i].width();
    }
    Int::Limits::double_check(c.max() * w * s.size(),
                              "Int::cumulative");
    if (home.failed()) return;

    int minU = INT_MAX; int minU2 = INT_MAX; int maxU = INT_MIN;
    for (int i=u.size(); i--;) {
      if (u[i] < minU)
        minU = u[i];
      else if (u[i] < minU2)
        minU2 = u[i];
      if (u[i] > maxU)
        maxU = u[i];
    }
    bool disjunctive =
      (minU > c.max()/2) || (minU2 > c.max()/2 && minU+minU2>c.max());
    if (disjunctive) {
      GECODE_ME_FAIL(c.gq(home,maxU));
      unary(home,t,s,p,icl);
    } else {
      bool fixp = true;
      for (int i=t.size(); i--;)
        if (t[i] != TT_FIXP) {
          fixp = false; break;
        }
      if (fixp) {
        TaskArray<ManFixPTask> tasks(home,s.size());
        for (int i=0; i<s.size(); i++)
          tasks[i].init(s[i],p[i],u[i]);
        GECODE_ES_FAIL((ManProp<ManFixPTask,Cap>::post(home,c,tasks)));
      } else {
        TaskArray<ManFixPSETask> tasks(home,s.size());
        for (int i=s.size(); i--;)
          tasks[i].init(t[i],s[i],p[i],u[i]);
        GECODE_ES_FAIL((ManProp<ManFixPSETask,Cap>::post(home,c,tasks)));
      }
    }
  }

  void
  cumulative(Home home, int c, const TaskTypeArgs& t,
             const IntVarArgs& s, const IntArgs& p, const IntArgs& u, 
             IntConLevel icl) {
    Int::Limits::nonnegative(c,"Int::cumulative");
    cumulative(home,Int::ConstIntView(c),t,s,p,u,icl);
  }
  void
  cumulative(Home home, IntVar c, const TaskTypeArgs& t,
             const IntVarArgs& s, const IntArgs& p, const IntArgs& u, 
             IntConLevel icl) {
    if (c.assigned())
      cumulative(home,c.val(),t,s,p,u,icl);
    else
      cumulative(home,Int::IntView(c),t,s,p,u,icl);
  }

  template<class Cap>
  void
  cumulative(Home home, Cap c, const TaskTypeArgs& t,
             const IntVarArgs& s, const IntArgs& p, const IntArgs& u,
             const BoolVarArgs& m, IntConLevel icl) {
    using namespace Gecode::Int;
    using namespace Gecode::Int::Cumulative;
    if ((s.size() != p.size()) || (s.size() != u.size()) ||
        (s.size() != t.size()) || (s.size() != m.size()))
      throw Int::ArgumentSizeMismatch("Int::cumulative");
    double w = 0.0;
    for (int i=p.size(); i--; ) {
      Int::Limits::nonnegative(p[i],"Int::cumulative");
      Int::Limits::nonnegative(u[i],"Int::cumulative");
      Int::Limits::check(static_cast<double>(s[i].max()) + p[i],
                         "Int::cumulative");
      Int::Limits::double_check(static_cast<double>(p[i]) * u[i],
                                "Int::cumulative");
      w += s[i].width();
    }
    Int::Limits::double_check(c.max() * w * s.size(),
                              "Int::cumulative");
    if (home.failed()) return;
    
    bool allMandatory = true;
    for (int i=m.size(); i--;) {
      if (!m[i].one()) {
        allMandatory = false;
        break;
      }
    }
    if (allMandatory) {
      cumulative(home,c,t,s,p,u,icl);
    } else {
      bool fixp = true;
      for (int i=t.size(); i--;)
        if (t[i] != TT_FIXP) {
          fixp = false; break;
        }
      if (fixp) {
        TaskArray<OptFixPTask> tasks(home,s.size());
        for (int i=0; i<s.size(); i++)
          tasks[i].init(s[i],p[i],u[i],m[i]);
        GECODE_ES_FAIL((OptProp<OptFixPTask,Cap>::post(home,c,tasks)));
      } else {
        TaskArray<OptFixPSETask> tasks(home,s.size());
        for (int i=s.size(); i--;)
          tasks[i].init(t[i],s[i],p[i],u[i],m[i]);
        GECODE_ES_FAIL((OptProp<OptFixPSETask,Cap>::post(home,c,tasks)));
      }
    }
  }
  
  void
  cumulative(Home home, int c, const TaskTypeArgs& t,
             const IntVarArgs& s, const IntArgs& p, const IntArgs& u,
             const BoolVarArgs& m, IntConLevel icl) {
    Int::Limits::nonnegative(c,"Int::cumulative");
    cumulative(home,Int::ConstIntView(c),t,s,p,u,m,icl);
  }
  void
  cumulative(Home home, IntVar c, const TaskTypeArgs& t,
             const IntVarArgs& s, const IntArgs& p, const IntArgs& u,
             const BoolVarArgs& m, IntConLevel icl) {
    if (c.assigned())
      cumulative(home,c.val(),t,s,p,u,m,icl);
    else
      cumulative(home,Int::IntView(c),t,s,p,u,m,icl);
  }
  
  template<class Cap>
  void
  cumulative(Home home, Cap c, const IntVarArgs& s, 
             const IntArgs& p, const IntArgs& u, IntConLevel icl) {
    using namespace Gecode::Int;
    using namespace Gecode::Int::Cumulative;
    if ((s.size() != p.size()) || (s.size() != u.size()))
      throw Int::ArgumentSizeMismatch("Int::cumulative");
    double w = 0.0;
    for (int i=p.size(); i--; ) {
      Int::Limits::nonnegative(p[i],"Int::cumulative");
      Int::Limits::nonnegative(u[i],"Int::cumulative");
      Int::Limits::check(static_cast<double>(s[i].max()) + p[i],
                         "Int::cumulative");
      Int::Limits::double_check(static_cast<double>(p[i]) * u[i],
                                "Int::cumulative");
      w += s[i].width();
    }
    Int::Limits::double_check(c.max() * w * s.size(),
                              "Int::cumulative");
    if (home.failed()) return;

    int minU = INT_MAX; int minU2 = INT_MAX; int maxU = INT_MIN;
    for (int i=u.size(); i--;) {
      if (u[i] < minU)
        minU = u[i];
      else if (u[i] < minU2)
        minU2 = u[i];
      if (u[i] > maxU)
        maxU = u[i];
    }
    bool disjunctive = 
      (minU > c.max()/2) || (minU2 > c.max()/2 && minU+minU2>c.max());
    if (disjunctive) {
      GECODE_ME_FAIL(c.gq(home,maxU));
      unary(home,s,p,icl);
    } else {
      TaskArray<ManFixPTask> t(home,s.size());
      for (int i=0; i<s.size(); i++) {
        t[i].init(s[i],p[i],u[i]);
      }
      GECODE_ES_FAIL((ManProp<ManFixPTask,Cap>::post(home,c,t)));
    }
  }

  void
  cumulative(Home home, int c, const IntVarArgs& s, 
             const IntArgs& p, const IntArgs& u, IntConLevel icl) {
    Int::Limits::nonnegative(c,"Int::cumulative");
    cumulative(home,Int::ConstIntView(c),s,p,u,icl);
  }
  void
  cumulative(Home home, IntVar c, const IntVarArgs& s, 
             const IntArgs& p, const IntArgs& u, IntConLevel icl) {
    if (c.assigned())
      cumulative(home,c.val(),s,p,u,icl);
    else
      cumulative(home,Int::IntView(c),s,p,u,icl);
  }
  
  template<class Cap>
  void
  cumulative(Home home, Cap c, const IntVarArgs& s, const IntArgs& p, 
             const IntArgs& u, const BoolVarArgs& m, IntConLevel icl) {
    using namespace Gecode::Int;
    using namespace Gecode::Int::Cumulative;
    if ((s.size() != p.size()) || (s.size() != u.size()) ||
        (s.size() != m.size()))
      throw Int::ArgumentSizeMismatch("Int::cumulative");
    double w = 0.0;
    for (int i=p.size(); i--; ) {
      Int::Limits::nonnegative(p[i],"Int::cumulative");
      Int::Limits::nonnegative(u[i],"Int::cumulative");
      Int::Limits::check(static_cast<double>(s[i].max()) + p[i],
                         "Int::cumulative");
      Int::Limits::double_check(static_cast<double>(p[i]) * u[i],
                                "Int::cumulative");
      w += s[i].width();
    }
    Int::Limits::double_check(c.max() * w * s.size(),
                              "Int::cumulative");
    if (home.failed()) return;

    bool allMandatory = true;
    for (int i=m.size(); i--;) {
      if (!m[i].one()) {
        allMandatory = false;
        break;
      }
    }
    if (allMandatory) {
      cumulative(home,c,s,p,u,icl);
    } else {
      TaskArray<OptFixPTask> t(home,s.size());
      for (int i=0; i<s.size(); i++) {
        t[i].init(s[i],p[i],u[i],m[i]);
      }
      GECODE_ES_FAIL((OptProp<OptFixPTask,Cap>::post(home,c,t)));
    }
  }

  void
  cumulative(Home home, int c, const IntVarArgs& s, const IntArgs& p, 
             const IntArgs& u, const BoolVarArgs& m, IntConLevel icl) {
    Int::Limits::nonnegative(c,"Int::cumulative");
    cumulative(home,Int::ConstIntView(c),s,p,u,m,icl);
  }
  void
  cumulative(Home home, IntVar c, const IntVarArgs& s, const IntArgs& p, 
             const IntArgs& u, const BoolVarArgs& m, IntConLevel icl) {
    if (c.assigned())
      cumulative(home,c.val(),s,p,u,m,icl);
    else
      cumulative(home,Int::IntView(c),s,p,u,m,icl);
  }

  template<class Cap>
  void
  cumulative(Home home, Cap c, const IntVarArgs& s, 
             const IntVarArgs& p, const IntVarArgs& e,
             const IntArgs& u, IntConLevel icl) {
    using namespace Gecode::Int;
    using namespace Gecode::Int::Cumulative;
    if ((s.size() != p.size()) || (s.size() != e.size()) ||
        (s.size() != u.size()))
      throw Int::ArgumentSizeMismatch("Int::cumulative");
    double w = 0.0;
    for (int i=p.size(); i--; ) {
      rel(home, p[i], IRT_GQ, 0);
    }
    for (int i=p.size(); i--; ) {
      Int::Limits::nonnegative(u[i],"Int::cumulative");
      Int::Limits::check(static_cast<double>(s[i].max()) + p[i].max(),
                         "Int::cumulative");
      Int::Limits::double_check(static_cast<double>(p[i].max()) * u[i],
                                "Int::cumulative");
      w += s[i].width();
    }
    Int::Limits::double_check(c.max() * w * s.size(),
                              "Int::cumulative");
    if (home.failed()) return;

    bool fixP = true;
    for (int i=p.size(); i--;) {
      if (!p[i].assigned()) {
        fixP = false;
        break;
      }
    }
    if (fixP) {
      IntArgs pp(p.size());
      for (int i=p.size(); i--;)
        pp[i] = p[i].val();
      cumulative(home,c,s,pp,u,icl);
    } else {
      TaskArray<ManFlexTask> t(home,s.size());
      for (int i=s.size(); i--; )
        t[i].init(s[i],p[i],e[i],u[i]);
      GECODE_ES_FAIL((ManProp<ManFlexTask,Cap>::post(home,c,t)));
    }
  }

  void
  cumulative(Home home, int c, const IntVarArgs& s, 
             const IntVarArgs& p, const IntVarArgs& e,
             const IntArgs& u, IntConLevel icl) {
    Int::Limits::nonnegative(c,"Int::cumulative");
    cumulative(home,Int::ConstIntView(c),s,p,e,u,icl);
  }
  void
  cumulative(Home home, IntVar c, const IntVarArgs& s, 
             const IntVarArgs& p, const IntVarArgs& e,
             const IntArgs& u, IntConLevel icl) {
    if (c.assigned())
      cumulative(home,c.val(),s,p,e,u,icl);
    else
      cumulative(home,Int::IntView(c),s,p,e,u,icl);
  }

  template<class Cap>
  void
  cumulative(Home home, Cap c, const IntVarArgs& s, const IntVarArgs& p,
             const IntVarArgs& e, const IntArgs& u, const BoolVarArgs& m, 
             IntConLevel icl) {
    using namespace Gecode::Int;
    using namespace Gecode::Int::Cumulative;
    if ((s.size() != p.size()) || (s.size() != u.size()) ||
        (s.size() != e.size()) || (s.size() != m.size()))
      throw Int::ArgumentSizeMismatch("Int::cumulative");
    for (int i=p.size(); i--; ) {
      rel(home, p[i], IRT_GQ, 0);
    }
    double w = 0.0;
    for (int i=p.size(); i--; ) {
      Int::Limits::nonnegative(u[i],"Int::cumulative");
      Int::Limits::check(static_cast<double>(s[i].max()) + p[i].max(),
                         "Int::cumulative");
      Int::Limits::double_check(static_cast<double>(p[i].max()) * u[i],
                                "Int::cumulative");
      w += s[i].width();
    }
    Int::Limits::double_check(c.max() * w * s.size(),
                              "Int::cumulative");
    if (home.failed()) return;

    bool allMandatory = true;
    for (int i=m.size(); i--;) {
      if (!m[i].one()) {
        allMandatory = false;
        break;
      }
    }
    if (allMandatory) {
      cumulative(home,c,s,p,e,u,icl);
    } else {
      TaskArray<OptFlexTask> t(home,s.size());
      for (int i=s.size(); i--; )
        t[i].init(s[i],p[i],e[i],u[i],m[i]);
      GECODE_ES_FAIL((OptProp<OptFlexTask,Cap>::post(home,c,t)));
    }
  }

  void
  cumulative(Home home, int c, const IntVarArgs& s, const IntVarArgs& p,
             const IntVarArgs& e, const IntArgs& u, const BoolVarArgs& m, 
             IntConLevel icl) {
    Int::Limits::nonnegative(c,"Int::cumulative");
    cumulative(home,Int::ConstIntView(c),s,p,e,u,m,icl);
  }
  void
  cumulative(Home home, IntVar c, const IntVarArgs& s, const IntVarArgs& p,
             const IntVarArgs& e, const IntArgs& u, const BoolVarArgs& m, 
             IntConLevel icl) {
    if (c.assigned())
      cumulative(home,c.val(),s,p,e,u,m,icl);
    else
      cumulative(home,Int::IntView(c),s,p,e,u,m,icl);
  }
  
}

// STATISTICS: int-post
