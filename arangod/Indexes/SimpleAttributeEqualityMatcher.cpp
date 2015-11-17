////////////////////////////////////////////////////////////////////////////////
/// @brief simple index attribute matcher
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2014 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Jan Steemann
/// @author Copyright 2014, ArangoDB GmbH, Cologne, Germany
/// @author Copyright 2011-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "SimpleAttributeEqualityMatcher.h"
#include "Aql/Ast.h"
#include "Aql/AstNode.h"
#include "Aql/Variable.h"
#include "Indexes/Index.h"
#include "VocBase/vocbase.h"

using namespace triagens::arango;

#include <iostream>
// -----------------------------------------------------------------------------
// --SECTION--                              class SimpleAttributeEqualityMatcher
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

SimpleAttributeEqualityMatcher::SimpleAttributeEqualityMatcher (std::vector<std::vector<triagens::basics::AttributeName>> const& attributes)
  : _attributes(attributes),
    _found() {
}
         
// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief match a single of the attributes
/// this is used for the primary index and the edge index
////////////////////////////////////////////////////////////////////////////////
        
bool SimpleAttributeEqualityMatcher::matchOne (triagens::arango::Index const* index,
                                               triagens::aql::AstNode const* node,
                                               triagens::aql::Variable const* reference,
                                               size_t itemsInIndex,
                                               size_t& estimatedItems,
                                               double& estimatedCost) {
  _found.clear();
            
  for (size_t i = 0; i < node->numMembers(); ++i) {
    auto op = node->getMember(i);

    if (op->type == triagens::aql::NODE_TYPE_OPERATOR_BINARY_EQ) {
      TRI_ASSERT(op->numMembers() == 2);
      // EQ is symmetric
      if (accessFitsIndex(index, op->getMember(0), op->getMember(1), op, reference, false) ||
          accessFitsIndex(index, op->getMember(1), op->getMember(0), op, reference, false)) {
        // we can use the index
        calculateIndexCosts(index, itemsInIndex, estimatedItems, estimatedCost);
        return true;
      }
    }
    else if (op->type == triagens::aql::NODE_TYPE_OPERATOR_BINARY_IN) {
      TRI_ASSERT(op->numMembers() == 2);
      if (accessFitsIndex(index, op->getMember(0), op->getMember(1), op, reference, false)) {
        // we can use the index
        // use slightly different cost calculation for IN that for EQ
        calculateIndexCosts(index, itemsInIndex, estimatedItems, estimatedCost);
        estimatedItems *= op->getMember(1)->numMembers();
        estimatedCost  *= op->getMember(1)->numMembers();
        return true;
      }
    }
  }

  // set to defaults
  estimatedItems = itemsInIndex;
  estimatedCost  = static_cast<double>(estimatedItems);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief match all of the attributes, in any order
/// this is used for the hash index
////////////////////////////////////////////////////////////////////////////////
        
bool SimpleAttributeEqualityMatcher::matchAll (triagens::arango::Index const* index,
                                               triagens::aql::AstNode const* node,
                                               triagens::aql::Variable const* reference,
                                               size_t itemsInIndex,
                                               size_t& estimatedItems,
                                               double& estimatedCost) {
  _found.clear();
  size_t values = 0;

  for (size_t i = 0; i < node->numMembers(); ++i) {
    auto op = node->getMember(i);

    if (op->type == triagens::aql::NODE_TYPE_OPERATOR_BINARY_EQ) {
      TRI_ASSERT(op->numMembers() == 2);

      if (accessFitsIndex(index, op->getMember(0), op->getMember(1), op, reference, false) ||
          accessFitsIndex(index, op->getMember(1), op->getMember(0), op, reference, false)) {
      }
    }
    else if (op->type == triagens::aql::NODE_TYPE_OPERATOR_BINARY_IN) {
      TRI_ASSERT(op->numMembers() == 2);

      if (accessFitsIndex(index, op->getMember(0), op->getMember(1), op, reference, false)) {
        auto m = op->getMember(1);

        if (m->isArray() && m->numMembers() > 1) {
          // attr IN [ a, b, c ]  =>  this will produce multiple items, so count them!
          values += m->numMembers() - 1;
        }
      }
    }
        
    if (_found.size() == _attributes.size()) {
      // got enough attributes
      break;
    }
  }

  if (_found.size() == _attributes.size()) {
    // can only use this index if all index attributes are covered by the condition
    if (values == 0) {
      values = 1;
    }

    calculateIndexCosts(index, itemsInIndex, estimatedItems, estimatedCost);
    estimatedItems *= values;
    estimatedCost  *= static_cast<double>(values);
    return true;
  }

  // set to defaults
  estimatedItems = itemsInIndex;
  estimatedCost  = static_cast<double>(estimatedItems);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief specialize the condition for the index
/// this is used for the primary index and the edge index
/// requires that a previous matchOne() returned true
////////////////////////////////////////////////////////////////////////////////

triagens::aql::AstNode* SimpleAttributeEqualityMatcher::specializeOne (triagens::arango::Index const* index,
                                                                       triagens::aql::AstNode* node,
                                                                       triagens::aql::Variable const* reference) {
  _found.clear();

  size_t const n = node->numMembers();

  for (size_t i = 0; i < n; ++i) {
    auto op = node->getMember(i);

    if (op->type == triagens::aql::NODE_TYPE_OPERATOR_BINARY_EQ) {
      TRI_ASSERT(op->numMembers() == 2);
      // EQ is symmetric
      if (accessFitsIndex(index, op->getMember(0), op->getMember(1), op, reference, false) ||
          accessFitsIndex(index, op->getMember(1), op->getMember(0), op, reference, false)) {
        // we can use the index
        // now return only the child node we need
        while (node->numMembers() > 0) {
          node->removeMemberUnchecked(0);
        }
        node->addMember(op);

        return node;
      }
    }
    else if (op->type == triagens::aql::NODE_TYPE_OPERATOR_BINARY_IN) {
      TRI_ASSERT(op->numMembers() == 2);

      if (accessFitsIndex(index, op->getMember(0), op->getMember(1), op, reference, false)) {
        // we can use the index
        // now return only the child node we need
        while (node->numMembers() > 0) {
          node->removeMemberUnchecked(0);
        }
        node->addMember(op);

        return node;
      }
    }
  }

  TRI_ASSERT(false);
  return node;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief specialize the condition for the index
/// this is used for the hash index
/// requires that a previous matchAll() returned true
////////////////////////////////////////////////////////////////////////////////

triagens::aql::AstNode* SimpleAttributeEqualityMatcher::specializeAll (triagens::arango::Index const* index,
                                                                       triagens::aql::AstNode* node,
                                                                       triagens::aql::Variable const* reference) {
  _found.clear();
  
  size_t const n = node->numMembers();

  for (size_t i = 0; i < n; ++i) {
    auto op = node->getMember(i);

    if (op->type == triagens::aql::NODE_TYPE_OPERATOR_BINARY_EQ) {
      TRI_ASSERT(op->numMembers() == 2);
      if (accessFitsIndex(index, op->getMember(0), op->getMember(1), op, reference, false) ||
          accessFitsIndex(index, op->getMember(1), op->getMember(0), op, reference, false)) {
        TRI_IF_FAILURE("SimpleAttributeMatcher::specializeAllChildrenEQ")  {
          THROW_ARANGO_EXCEPTION(TRI_ERROR_DEBUG);
        }
        if (_found.size() == _attributes.size()) {
          // got enough attributes
          break;
        }
      }
    }
    else if (op->type == triagens::aql::NODE_TYPE_OPERATOR_BINARY_IN) {
      TRI_ASSERT(op->numMembers() == 2);
      if (accessFitsIndex(index, op->getMember(0), op->getMember(1), op, reference, false)) {
        TRI_IF_FAILURE("SimpleAttributeMatcher::specializeAllChildrenIN")  {
          THROW_ARANGO_EXCEPTION(TRI_ERROR_DEBUG);
        }
        if (_found.size() == _attributes.size()) {
          // got enough attributes
          break;
        }
      }
    }
  }

  if (_found.size() == _attributes.size()) {
    // remove node's existing members
    while (node->numMembers() > 0) {
      node->removeMemberUnchecked(0);
    }
    // found contains all nodes required for this condition sorted by _attributes
    // now re-add only those
    for (size_t i = 0; i < _attributes.size(); ++i) {
      // This is always save due to
      auto it = _found.find(i);
      TRI_ASSERT(it != _found.end()); // Found contains by def. 1 Element for each _attribute
      node->addMember(it->second);
    }

    return node;
  }

  TRI_ASSERT(false);
  return node;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief determine the costs of using this index and the number of items
/// that will return in average
/// cost values have no special meaning, except that multiple cost values are 
/// comparable, and lower values mean lower costs
////////////////////////////////////////////////////////////////////////////////

void SimpleAttributeEqualityMatcher::calculateIndexCosts (triagens::arango::Index const* index,
                                                          size_t itemsInIndex,
                                                          size_t& estimatedItems,
                                                          double& estimatedCost) const {
  double equalityReductionFactor = 20.0;

  if (index->unique()) {
    // index is unique, and the condition covers all attributes
    // now use a low value for the costs
    estimatedItems = 1;
    estimatedCost = 1.0;
  }
  else if (index->hasSelectivityEstimate()) {
    // use index selectivity estimate
    double estimate = index->selectivityEstimate();
    if (estimate <= 0.0) {
      // prevent division by zero
      estimatedItems = itemsInIndex;
      // the more attributes are contained in the index, the more specific the lookup will be
      for (size_t i = 0; i < index->fields().size(); ++i) {
        estimatedItems /= static_cast<size_t>(equalityReductionFactor);
        // decrease the effect of the equality reduction factor
        equalityReductionFactor *= 0.25; 
        if (equalityReductionFactor < 2.0) {
          // equalityReductionFactor shouldn't get too low
          equalityReductionFactor = 2.0; 
        }
      }
    }
    else {
      estimatedItems = static_cast<size_t>(1.0 / estimate);
    }

    estimatedItems = (std::max)(estimatedItems, static_cast<size_t>(1));
    // the more attributes are covered by an index, the more accurate it
    // is considered to be
    estimatedCost = static_cast<double>(estimatedItems) - index->fields().size() * 0.01;
  }
  else {
    // no such index should exist
    TRI_ASSERT(false);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not the access fits
////////////////////////////////////////////////////////////////////////////////

bool SimpleAttributeEqualityMatcher::accessFitsIndex (triagens::arango::Index const* index,
                                                      triagens::aql::AstNode const* access,
                                                      triagens::aql::AstNode const* other,
                                                      triagens::aql::AstNode const* op,
                                                      triagens::aql::Variable const* reference,
                                                      bool isExecution) {
  if (! index->canUseConditionPart(access, other, op, reference, isExecution)) {
    return false;
  }

  triagens::aql::AstNode const* what = access;
  std::pair<triagens::aql::Variable const*, std::vector<triagens::basics::AttributeName>> attributeData;

  if (op->type != triagens::aql::NODE_TYPE_OPERATOR_BINARY_IN) {
    if (! what->isAttributeAccessForVariable(attributeData) ||
        attributeData.first != reference) {
        // this access is not referencing this collection
        return false;
    }
    if (triagens::basics::TRI_AttributeNamesHaveExpansion(attributeData.second)) {
      // doc.value[*] IN 'value'
      return false;
    }
  }
  else { 
    // ok, we do have an IN here... check if it's something like 'value' IN doc.value[*]
    bool canUse = false;
    if (what->isAttributeAccessForVariable(attributeData) && 
        attributeData.first == reference &&
        ! triagens::basics::TRI_AttributeNamesHaveExpansion(attributeData.second)) {
      // doc.value[*] IN 'value'
      canUse = true;
    }
    if (! canUse) {
      // check for doc.value[*] IN 'value'
      what = other;
      if (! what->isAttributeAccessForVariable(attributeData) ||
          attributeData.first != reference) {
        // this access is not referencing this collection
        return false;
      }
    }
  }

  std::vector<triagens::basics::AttributeName> const& fieldNames = attributeData.second;

  for (size_t i = 0; i < _attributes.size(); ++i) {
    if (_attributes[i].size() != fieldNames.size()) {
      // attribute path length differs
      continue;
    }
    if (index->isAttributeExpanded(i) &&
        op->type != triagens::aql::NODE_TYPE_OPERATOR_BINARY_IN) {
      // If this attribute is correct or not, it could only serve for IN
      continue;
    }

    bool match = true;
    for (size_t j = 0; j < _attributes[i].size(); ++j) {
      if (_attributes[i][j] != fieldNames[j]) {
        // special case: a[*] is identical to a, and a.b[*] is identical to a.b
        // general rule: if index attribute is expanded and last part in index, then it can
        // be used in a query without expansion operator
        bool const isLast = (j == _attributes[i].size() - 1);

        if (! isLast || (! _attributes[i][j].shouldExpand) || _attributes[i][j].name != fieldNames[j].name) {
          match = false;
          break;
        }
      }
    }

    if (match) {
      // mark ith attribute as being covered
      _found.emplace(i, op);
      TRI_IF_FAILURE("SimpleAttributeMatcher::accessFitsIndex")  {
        THROW_ARANGO_EXCEPTION(TRI_ERROR_DEBUG);
      }
      return true;
    }
  }

  return false;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
