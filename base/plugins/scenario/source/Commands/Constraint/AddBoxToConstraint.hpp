#pragma once
#include <core/presenter/command/SerializableCommand.hpp>
#include <tools/ObjectPath.hpp>

class AddBoxToConstraintTest;
class AddProcessViewModelToDeckTest;
class HideBoxInViewModelTest;
namespace Scenario
{
	namespace Command
	{
		/**
		 * @brief The AddBoxToConstraint class
		 *
		 * Adds an empty box, with no decks, to a constraint.
		 */
		class AddBoxToConstraint : public iscore::SerializableCommand
		{
				friend class ::AddProcessViewModelToDeckTest;
				friend class ::AddBoxToConstraintTest;
				friend class ::HideBoxInViewModelTest;
			public:
				AddBoxToConstraint(ObjectPath&& constraintPath);

				virtual void undo() override;
				virtual void redo() override;
				virtual int id() const override;
				virtual bool mergeWith(const QUndoCommand* other) override;

			protected:
				virtual void serializeImpl(QDataStream&) override;
				virtual void deserializeImpl(QDataStream&) override;

			private:
				ObjectPath m_path;

				int m_createdBoxId{};
		};
	}
}
